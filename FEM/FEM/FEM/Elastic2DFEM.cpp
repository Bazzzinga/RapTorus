#include "Elastic2DFEM.h"

#include <iostream>

Elastic2DFEM::Elastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp) : FEM(_dba), elementHeight(h)
{
	pProp = _pProp;

	double multiplierEM = pProp.E / (1 - pProp.v * pProp.v);

	elasticMatrix << 1.0, pProp.v, 0.0, pProp.v, 1.0, 0.0, 0.0, 0.0, (1.0 - pProp.v) / 2.0;

	elasticMatrix *= multiplierEM;

	rightSide.resize(2 * (int)NodeList.size());
	rightSide.setZero();

	initialDisplacement.resize(2 * (int)NodeList.size());
	initialDisplacement.setZero();

	loads.resize(2 * (int)NodeList.size());
	loads.setZero();
}

Elastic2DFEM::~Elastic2DFEM()
{

}


Eigen::Matrix2d Elastic2DFEM::Jacobian4(VectorXd x1, VectorXd x2, double s, double t)
{
	Eigen::Matrix2d res;

	res(0, 0) = (x1(0) * (t - 1) + x1(1) * (1 - t) + x1(2) * (1 + t) + x1(3) * (-1 - t)) / 4.0;
	res(1, 0) = (x1(0) * (s - 1) + x1(1) * (-1 - s) + x1(2) * (1 + s) + x1(3) * (1 - s)) / 4.0;
	res(0, 1) = (x2(0) * (t - 1) + x2(1) * (1 - t) + x2(2) * (1 + t) + x2(3) * (-1 - t)) / 4.0;
	res(1, 1) = (x2(0) * (s - 1) + x2(1) * (-1 - s) + x2(2) * (1 + s) + x2(3) * (1 - s)) / 4.0;

	return res;
}
	
void Elastic2DFEM::Solve(void)
{
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(StiffnessMatrix);

	displacements = solver.solve(loads + rightSide);
	
	for (int i = 0; i < (int)NodeList.size(); ++i)
		if ((initialDisplacement(2 * i) != 0) || (initialDisplacement(2 * i + 1) != 0))
		{
			displacements(2 * i) = initialDisplacement(2 * i);
			displacements(2 * i + 1) = initialDisplacement(2 * i + 1);
		}

	PushDisplacementsToDB();

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
	{
		int nodeCount = (int)it->nodeIDs.size();

		Eigen::MatrixXd delta;

		delta.resize(2 * nodeCount, 1);

		for (int nid = 0; nid < nodeCount; ++nid)
		{
			Eigen::MatrixXd temp;
			temp.resize(2, 1);

			temp << displacements.segment<2>(2 * getNodeIndex(it->nodeIDs[nid]));

			delta(2 * nid, 0)		= temp(0, 0);
			delta(2 * nid + 1, 0)	= temp(1, 0);
		}
		
		Eigen::Vector3d sigma = elasticMatrix * it->B * delta;

		it->sigma_mises = sqrt(sigma[0] * sigma[0] - sigma[0] * sigma[1] + sigma[1] * sigma[1] + 3.0f * sigma[2] * sigma[2]);
	}

	PushSigmaMisesToDB(false);
}


void Elastic2DFEM::SetConstraintsFromList(std::vector<_ConstraintsS> constraint)
{
	std::vector<int> indicesToConstraint;

	for (std::vector<_ConstraintsS>::const_iterator it = constraint.begin(); it != constraint.end(); ++it)
	{
		if (it->type & _ConstraintsS::UX1)
		{
			indicesToConstraint.push_back(2 * it->nodeID + 0);
		}
		if (it->type & _ConstraintsS::UX2)
		{
			indicesToConstraint.push_back(2 * it->nodeID + 1);
		}
	}

	for (int k = 0; k < StiffnessMatrix.outerSize(); ++k)
	{
		for (Eigen::SparseMatrix<double>::InnerIterator it(StiffnessMatrix, k); it; ++it)
		{
			for (std::vector<int>::iterator idit = indicesToConstraint.begin(); idit != indicesToConstraint.end(); ++idit)
			{
				if (it.row() == *idit || it.col() == *idit)
				{
					it.valueRef() = it.row() == it.col() ? 1.0f : 0.0f;
				}
			}
		}
	}
}

void Elastic2DFEM::SetLoadsFromList(std::vector<_Load2DS> loadList)
{
	for (std::vector<_Load2DS>::const_iterator it = loadList.begin(); it != loadList.end(); ++it)
	{
		loads[2 * it->nodeID] += it->forceX1;
		loads[2 * it->nodeID + 1] += it->forceX2;
	}
}


void Elastic2DFEM::SetConstraints(int NSID, _ConstraintsS::Type type)
{
	std::vector<_ConstraintsS> constraint;

	_ConstraintsS tempConstr;

	tempConstr.type = type;

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].SetID == NSID)
		{
			NSindex = j;
			break;
		}
	if (NamedSets[NSindex].type == NAMED_SET_TYPE_NODE)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			tempConstr.nodeID = getNodeIndex(NamedSets[NSindex].itemList[i]);
			constraint.push_back(tempConstr);
		}
	else if (NamedSets[NSindex].type == NAMED_SET_TYPE_ELEM)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			int index = getElemIndex(NamedSets[NSindex].itemList[i]);

			for (int j = 0; j < (int)ElementList[index].nodeIDList.size(); ++j)
			{
				tempConstr.nodeID = getNodeIndex(ElementList[index].nodeIDList[j]);
				constraint.push_back(tempConstr);
			}
		}

	SetConstraintsFromList(constraint);
}

void Elastic2DFEM::SetConstraints(std::string NSName, _ConstraintsS::Type type)
{
	std::vector<_ConstraintsS> constraint;

	_ConstraintsS tempConstr;

	tempConstr.type = type;

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.compare(NSName) == 0)
		{
			NSindex = j;
			break;
		}
	if (NamedSets[NSindex].type == NAMED_SET_TYPE_NODE)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			tempConstr.nodeID = getNodeIndex(NamedSets[NSindex].itemList[i]);
			constraint.push_back(tempConstr);
		}
	else if (NamedSets[NSindex].type == NAMED_SET_TYPE_ELEM)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			int index = getElemIndex(NamedSets[NSindex].itemList[i]);

			for (int j = 0; j < (int)ElementList[index].nodeIDList.size(); ++j)
			{
				tempConstr.nodeID = getNodeIndex(ElementList[index].nodeIDList[j]);
				constraint.push_back(tempConstr);
			}
		}
	SetConstraintsFromList(constraint);
}

void Elastic2DFEM::SetLoads(int NSID, Eigen::VectorXd force)
{
	std::vector<_Load2DS> loadList;

	_Load2DS tempLoad;

	tempLoad.forceX1 = force[0];
	tempLoad.forceX2 = force[1];

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].SetID == NSID)
		{
			NSindex = j;
			break;
		}

	if (NamedSets[NSindex].type == NAMED_SET_TYPE_NODE)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			tempLoad.nodeID = getNodeIndex(NamedSets[NSindex].itemList[i]);
			loadList.push_back(tempLoad);
		}
	else
		return;

	SetLoadsFromList(loadList);
}

void Elastic2DFEM::SetLoads(std::string NSName, Eigen::VectorXd force)
{
	std::vector<_Load2DS> loadList;

	_Load2DS tempLoad;

	tempLoad.forceX1 = force[0];
	tempLoad.forceX2 = force[1];

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.compare(NSName) == 0)
		{
			NSindex = j;
			break;
		}

	if (NamedSets[NSindex].type == NAMED_SET_TYPE_NODE)
		for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
		{
			tempLoad.nodeID = getNodeIndex(NamedSets[NSindex].itemList[i]);
			loadList.push_back(tempLoad);
		}
	else
		return;

	SetLoadsFromList(loadList);
}


void Elastic2DFEM::SetDisplacement(int NSID, Eigen::VectorXd disp)
{
	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].SetID == NSID)
		{
			NSindex = j;
			break;
		}

	for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
	{
		int index = getNodeIndex(NamedSets[NSindex].itemList[i]);

		std::vector<_ConstraintsS> constraint;

		_ConstraintsS temp;

		temp.type = _ConstraintsS::UX1X2;
		temp.nodeID = index;

		constraint.push_back(temp);

		if (disp[0] != 0.0)
		{
			VectorXd x1col = StiffnessMatrix.col(2 * index);

			x1col *= disp[0];

			initialDisplacement(2 * index) = disp[0];

			rightSide -= x1col;
		}

		if (disp[1] != 0.0)
		{
			VectorXd x2col = StiffnessMatrix.col(2 * index + 1);

			x2col *= disp[1];

			initialDisplacement(2 * index + 1) = disp[1];

			rightSide -= x2col;
		}

		SetConstraintsFromList(constraint);
	}
}

void Elastic2DFEM::SetDisplacement(std::string NSName, Eigen::VectorXd disp)
{
	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.compare(NSName) == 0)
		{
			NSindex = j;
			break;
		}

	for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
	{
		int index = getNodeIndex(NamedSets[NSindex].itemList[i]);

		std::vector<_ConstraintsS> constraint;

		_ConstraintsS temp;

		temp.type = _ConstraintsS::UX1X2;
		temp.nodeID = index;

		constraint.push_back(temp);

		if (disp[0] != 0.0)
		{
			VectorXd x1col = StiffnessMatrix.col(2 * index);

			x1col *= disp[0];

			initialDisplacement(2 * index) = disp[0];

			rightSide -= x1col;
		}

		if (disp[1] != 0.0)
		{
			VectorXd x2col = StiffnessMatrix.col(2 * index + 1);

			x2col *= disp[1];

			initialDisplacement(2 * index + 1) = disp[1];

			rightSide -= x2col;
		}

		SetConstraintsFromList(constraint);
	}
}