#include "Elastic3DFEM.h"

#include <iostream>
#include <fstream>

Elastic3DFEM::Elastic3DFEM(DatabaseAgent * _dba, _PhysicalProperetiesS _pProp) : FEM(_dba)
{
	pProp = _pProp;

	elasticMatrix.resize(6, 6);
	
	double multiplierEM = pProp.E / (1.0 + pProp.v) / (1 - 2.0 * pProp.v);

	elasticMatrix <<	1.0 - pProp.v, pProp.v, pProp.v, 0, 0, 0,
						pProp.v, 1.0 - pProp.v, pProp.v, 0, 0, 0,
						pProp.v, pProp.v, 1.0 - pProp.v, 0, 0, 0,
						0, 0, 0, (1.0 - 2.0 * pProp.v) / 2.0, 0, 0,
						0, 0, 0, 0, (1.0 - 2.0 * pProp.v) / 2.0, 0,
						0, 0, 0, 0, 0, (1.0 - 2.0 * pProp.v) / 2.0;

	elasticMatrix *= multiplierEM;	
	// -1 < v < 0.5 !!
	
	rightSide.resize(3 * (int)NodeList.size());
	rightSide.setZero();

	initialDisplacement.resize(3 * (int)NodeList.size());
	initialDisplacement.setZero();

	loads.resize(3 * (int)NodeList.size());
	loads.setZero();
}

Elastic3DFEM::~Elastic3DFEM()
{

}

void Elastic3DFEM::Solve(void)
{
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(StiffnessMatrix);

	displacements = solver.solve(loads + rightSide);
	
	Eigen::VectorXd mult = StiffnessMatrix * displacements;

	std::filebuf fb;
	fb.open("S1.dat", std::ios::out);
	std::filebuf fb2;
	fb2.open("R1.dat", std::ios::out);
	std::filebuf fb4;
	fb4.open("L1.dat", std::ios::out);

	std::ostream os(&fb);
	for (int k = 0; k < StiffnessMatrix.outerSize(); ++k)
		for (Eigen::SparseMatrix<double>::InnerIterator it(StiffnessMatrix, k); it; ++it)
			os << it.col() + 1 << " " << it.row() + 1 << " " << it.value() << std::endl;
	std::ostream os2(&fb2);
	os2 << (rightSide);

	std::ostream os4(&fb4);
	os4 << (loads);

	fb.close();
	fb2.close();
	fb4.close();
	

	for (int i = 0; i < (int)NodeList.size(); ++i)
		if ((initialDisplacement(3 * i) != 0) || (initialDisplacement(3 * i + 1) != 0) || (initialDisplacement(3 * i + 2) != 0))
		{
			displacements(3 * i) = initialDisplacement(3 * i);
			displacements(3 * i + 1) = initialDisplacement(3 * i + 1);
			displacements(3 * i + 2) = initialDisplacement(3 * i + 2);
		}
	std::filebuf fb3;
	fb3.open("D1.dat", std::ios::out);

	std::ostream os3(&fb3);

	os3 << displacements;

	fb3.close();


	PushDisplacementsToDB();

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
	{
		int nodeCount = (int)it->nodeIDs.size();

		Eigen::MatrixXd delta;

		delta.resize(3 * nodeCount, 1);

		for (int nid = 0; nid < nodeCount; ++nid)
		{
			Eigen::MatrixXd temp;
			temp.resize(3, 1);

			temp << displacements.segment<3>(3 * getNodeIndex(it->nodeIDs[nid]));

			delta(3 * nid, 0) = temp(0, 0);
			delta(3 * nid + 1, 0) = temp(1, 0);
			delta(3 * nid + 2, 0) = temp(2, 0);
		}

		Eigen::VectorXd sigma = elasticMatrix * it->B * delta;

		it->sigma_mises  = (sigma[0] * sigma[0] - sigma[1] * sigma[1]) * (sigma[0] * sigma[0] - sigma[1] * sigma[1]);
		it->sigma_mises += (sigma[1] * sigma[1] - sigma[2] * sigma[2]) * (sigma[1] * sigma[1] - sigma[2] * sigma[2]);
		it->sigma_mises += (sigma[2] * sigma[2] - sigma[0] * sigma[0]) * (sigma[2] * sigma[2] - sigma[0] * sigma[0]);
		it->sigma_mises += 6.0 * (sigma[3] * sigma[3] + sigma[4] * sigma[4] + sigma[5] * sigma[5]);
		it->sigma_mises /= 2.0;
		it->sigma_mises = sqrt(it->sigma_mises);
	}

	PushSigmaMisesToDB(true);
}

double Elastic3DFEM::formFunctionDeriv(int index, int denom, double s, double t, double u)
{		
	double multiplier = 0.125; // 1/8

	int k1 = formFunctionDerivCoeff[index][denom * 4];
	int k2 = formFunctionDerivCoeff[index][denom * 4 + 1];
	int k3 = formFunctionDerivCoeff[index][denom * 4 + 2];
	int k4 = formFunctionDerivCoeff[index][denom * 4 + 3];

	double res = k1 * multiplier *(1.0 + k2 * s) * (1.0 + k3 * t) * (1.0 + k4 * u);

	return res;
}

Eigen::Matrix3d Elastic3DFEM::Jacobian8(VectorXd x1, VectorXd x2, VectorXd x3, double s, double t, double u)
{
	Eigen::Matrix3d res;

	double temp00 = 0.0, temp01 = 0.0, temp02 = 0.0;
	double temp10 = 0.0, temp11 = 0.0, temp12 = 0.0;
	double temp20 = 0.0, temp21 = 0.0, temp22 = 0.0;

	
	for (int i = 0; i < x1.size(); ++i)
	{
		temp00 += x1(i) * formFunctionDeriv(i, 0, s, t, u);
		temp01 += x2(i) * formFunctionDeriv(i, 0, s, t, u);
		temp02 += x3(i) * formFunctionDeriv(i, 0, s, t, u);

		temp10 += x1(i) * formFunctionDeriv(i, 1, s, t, u);
		temp11 += x2(i) * formFunctionDeriv(i, 1, s, t, u);
		temp12 += x3(i) * formFunctionDeriv(i, 1, s, t, u);

		temp20 += x1(i) * formFunctionDeriv(i, 2, s, t, u);
		temp21 += x2(i) * formFunctionDeriv(i, 2, s, t, u);
		temp22 += x3(i) * formFunctionDeriv(i, 2, s, t, u);
	}
	
	res <<	temp00, temp01, temp02,
			temp10, temp11, temp12,
			temp20, temp21, temp22;

	return res;
}

Eigen::Matrix4d Elastic3DFEM:: Jacobian4(VectorXd x1, VectorXd x2, VectorXd x3)
{
	Eigen::Matrix4d res;

	res << 	1, 1, 1, 1,
			x1[0], x1[1], x1[2], x1[3],
			x2[0], x2[1], x2[2], x2[3],
			x3[0], x3[1], x3[2], x3[3];

	return res;
}


void Elastic3DFEM::SetConstraintsFromList(std::vector<_ConstraintsS> constraint)
{
	std::vector<int> indicesToConstraint;

	for (std::vector<_ConstraintsS>::const_iterator it = constraint.begin(); it != constraint.end(); ++it)
	{
		if (it->type & _ConstraintsS::UX1)
		{
			indicesToConstraint.push_back(3 * it->nodeID + 0);
		}
		if (it->type & _ConstraintsS::UX2)
		{
			indicesToConstraint.push_back(3 * it->nodeID + 1);
		}
		if (it->type & _ConstraintsS::UX3)
		{
			indicesToConstraint.push_back(3 * it->nodeID + 2);
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

void Elastic3DFEM::SetLoadsFromList(std::vector<_Load3DS> loadList)
{
	for (std::vector<_Load3DS>::const_iterator it = loadList.begin(); it != loadList.end(); ++it)
	{
		loads[3 * it->nodeID] += it->forceX1;
		loads[3 * it->nodeID + 1] += it->forceX2;
		loads[3 * it->nodeID + 2] += it->forceX3;
	}
}


void Elastic3DFEM::SetConstraints(int NSID, _ConstraintsS::Type type)
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

void Elastic3DFEM::SetConstraints(std::string NSName, _ConstraintsS::Type type)
{
	std::vector<_ConstraintsS> constraint;

	_ConstraintsS tempConstr;

	tempConstr.type = type;

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.find(NSName) == 0)
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

void Elastic3DFEM::SetLoads(int NSID, Eigen::VectorXd force)
{
	std::vector<_Load3DS> loadList;

	_Load3DS tempLoad;

	tempLoad.forceX1 = force[0];
	tempLoad.forceX2 = force[1];
	tempLoad.forceX3 = force[2];

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

void Elastic3DFEM::SetLoads(std::string NSName, Eigen::VectorXd force)
{
	std::vector<_Load3DS> loadList;

	_Load3DS tempLoad;

	tempLoad.forceX1 = force[0];
	tempLoad.forceX2 = force[1];
	tempLoad.forceX3 = force[2];

	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.find(NSName) == 0)
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


void Elastic3DFEM::SetDisplacement(int NSID, Eigen::VectorXd disp)
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
			VectorXd x1col = StiffnessMatrix.col(3 * index);

			x1col *= disp[0];

			initialDisplacement(3 * index) = disp[0];

			rightSide -= x1col;
		}

		if (disp[1] != 0.0)
		{
			VectorXd x2col = StiffnessMatrix.col(3 * index + 1);

			x2col *= disp[1];

			initialDisplacement(3 * index + 1) = disp[1];

			rightSide -= x2col;
		}

		if (disp[2] != 0.0)
		{
			VectorXd x3col = StiffnessMatrix.col(3 * index + 2);

			x3col *= disp[1];

			initialDisplacement(3 * index + 2) = disp[3];

			rightSide -= x3col;
		}

		SetConstraintsFromList(constraint);
	}
}

void Elastic3DFEM::SetDisplacement(std::string NSName, Eigen::VectorXd disp)
{
	int NSindex = 0;

	for (int j = 0; j < (int)NamedSets.size(); ++j)
		if (NamedSets[j].name.find(NSName) == 0)
		{
			NSindex = j;
			break;
		}

	for (int i = 0; i < (int)NamedSets[NSindex].itemList.size(); ++i)
	{
		int index = getNodeIndex(NamedSets[NSindex].itemList[i]);

		std::vector<_ConstraintsS> constraint;

		_ConstraintsS temp;

		temp.type = _ConstraintsS::UX1X2X3;
		temp.nodeID = index;

		constraint.push_back(temp);

		if (disp[0] != 0.0)
		{
			VectorXd x1col = StiffnessMatrix.col(3 * index);

			x1col *= disp[0];

			initialDisplacement(3 * index) = disp[0];

			rightSide -= x1col;
		}

		if (disp[1] != 0.0)
		{
			VectorXd x2col = StiffnessMatrix.col(3 * index + 1);

			x2col *= disp[1];

			initialDisplacement(3 * index + 1) = disp[1];

			rightSide -= x2col;
		}

		if (disp[2] != 0.0)
		{
			VectorXd x3col = StiffnessMatrix.col(3 * index + 2);

			x3col *= disp[2];

			initialDisplacement(3 * index + 2) = disp[2];

			rightSide -= x3col;
		}

		SetConstraintsFromList(constraint);
	}
}