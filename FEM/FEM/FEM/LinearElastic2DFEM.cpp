#include "LinearElastic2DFEM.h"

LinearElastic2DFEM::LinearElastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp) : Elastic2DFEM(_dba, h, _pProp)
{
	std::vector<Eigen::Triplet<double>> triplets;

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
		calcMatrixes(*it, elasticMatrix, triplets);

	StiffnessMatrix.resize(2 * (int)NodeList.size(), 2 * (int)NodeList.size());
	StiffnessMatrix.setFromTriplets(triplets.begin(), triplets.end());
}

LinearElastic2DFEM::~LinearElastic2DFEM()
{

}


Matrix3Xd LinearElastic2DFEM::calcB3(VectorXd x1, VectorXd x2)
{
	Matrix3Xd B;

	B.resize(3, 2 * (int)x1.size());

	Eigen::Matrix3d J, _J;

	_J << 1, 1, 1,
		x1(0), x1(1), x1(2),
		x2(0), x2(1), x2(2);

	J = _J.inverse();

	for (int jj = 0; jj < (int)x1.size(); ++jj)
	{
		B(0, 2 * jj + 0) = J(jj, 1);
		B(0, 2 * jj + 1) = 0.0;
		B(1, 2 * jj + 0) = 0.0;
		B(1, 2 * jj + 1) = J(jj, 2);
		B(2, 2 * jj + 0) = J(jj, 2);
		B(2, 2 * jj + 1) = J(jj, 1);
	}

	return B;
}

Matrix3Xd LinearElastic2DFEM::calcB4(VectorXd x1, VectorXd x2, double s, double t)
{
	Matrix3Xd B;

	B.resize(3, 2 * (int)x1.size());

	for (int jj = 0; jj < (int)x1.size(); ++jj)
	{
		Eigen::Vector2d xy_dN, st_dN;

		switch (jj)
		{
		case 0:
			st_dN(0) = (-1 + t) / 4.0;
			st_dN(1) = (-1 + s) / 4.0;
			break;
		case 1:
			st_dN(0) = (1 - t) / 4.0;
			st_dN(1) = (-1 - s) / 4.0;
			break;
		case 2:
			st_dN(0) = (1 + t) / 4.0;
			st_dN(1) = (1 + s) / 4.0;
			break;
		case 3:
			st_dN(0) = (-1 - t) / 4.0;
			st_dN(1) = (1 - s) / 4.0;
			break;
		}

		xy_dN = Jacobian4(x1, x2, s, t).inverse() * st_dN;

		B(0, 2 * jj + 0) = xy_dN(0);
		B(0, 2 * jj + 1) = 0.0;
		B(1, 2 * jj + 0) = 0.0;
		B(1, 2 * jj + 1) = xy_dN(1);
		B(2, 2 * jj + 0) = xy_dN(1);
		B(2, 2 * jj + 1) = xy_dN(0);
	}

	return B;
}

void LinearElastic2DFEM::calcMatrixes(FEMElement& FEMelem, const Eigen::Matrix3d& D, std::vector<Eigen::Triplet<double> >& triplets)
{
	VectorXd Nodex1;
	VectorXd Nodex2;

	std::vector<int> NodeIndexes;

	MatrixXXd K;

	K.resize(2 * FEMelem.nodeCount, 2 * FEMelem.nodeCount);

	K.setZero();

	Nodex1.resize(FEMelem.nodeCount);
	Nodex2.resize(FEMelem.nodeCount);

	for (int k = 0; k < FEMelem.nodeCount; ++k)
	{
		int index = getNodeIndex(FEMelem.nodeIDs[k]);
		NodeIndexes.push_back(index);

		Nodex1(k) = NodeList[index].x[0];
		Nodex2(k) = NodeList[index].x[1];
	}

	FEMelem.S = calcElementArea(FEMelem.elemID);

	if (FEMelem.nodeCount == 3)
	{
		FEMelem.B = calcB3(Nodex1, Nodex2);

		K += FEMelem.S * FEMelem.B.transpose() * D * FEMelem.B * elementHeight;
	}
	else if (FEMelem.nodeCount == 4)
	{
		double gaussPoint = 1 / sqrt(3);

		for (int i = -1; i <= 1; i += 2)
			for (int j = -1; j <= 1; j += 2)
			{
				Eigen::Matrix3Xd BB = calcB4(Nodex1, Nodex2, gaussPoint * i, gaussPoint * j);

				K += Jacobian4(Nodex1, Nodex2, gaussPoint * i, gaussPoint * j).determinant() * BB.transpose() * D * BB;
			}

		FEMelem.B = calcB4(Nodex1, Nodex2, 0.0, 0.0);

		K *= 4.0 * elementHeight;
	}

	for (int i = 0; i < FEMelem.nodeCount; ++i)
	{
		for (int j = 0; j < FEMelem.nodeCount; ++j)
		{
			Eigen::Triplet<double> trpl11(2 * NodeIndexes[i] + 0, 2 * NodeIndexes[j] + 0, K(2 * i + 0, 2 * j + 0));
			Eigen::Triplet<double> trpl12(2 * NodeIndexes[i] + 0, 2 * NodeIndexes[j] + 1, K(2 * i + 0, 2 * j + 1));
			Eigen::Triplet<double> trpl21(2 * NodeIndexes[i] + 1, 2 * NodeIndexes[j] + 0, K(2 * i + 1, 2 * j + 0));
			Eigen::Triplet<double> trpl22(2 * NodeIndexes[i] + 1, 2 * NodeIndexes[j] + 1, K(2 * i + 1, 2 * j + 1));

			triplets.push_back(trpl11);
			triplets.push_back(trpl12);
			triplets.push_back(trpl21);
			triplets.push_back(trpl22);
		}
	}
}


double LinearElastic2DFEM::calcElementArea(int elemID)
{
	VectorXd Nodex1;
	VectorXd Nodex2;

	double area1 = 0;
	double area2 = 0;

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
		if (it->elemID == elemID)
		{
			Nodex1.resize(it->nodeCount);
			Nodex2.resize(it->nodeCount);


			for (int k = 0; k < it->nodeCount; ++k)
			{
				int index = getNodeIndex(it->nodeIDs[k]);

				Nodex1(k) = NodeList[index].x[0];
				Nodex2(k) = NodeList[index].x[1];
			}

			for (int ii = 0; ii < it->nodeCount; ++ii)
			{
				int kk = ii + 1;
				if (kk == it->nodeCount)
					kk = 0;

				area1 += Nodex1[ii] * Nodex2[kk];
				area2 += Nodex1[kk] * Nodex2[ii];
			}

			return (area1 - area2) / 2.0;
		}
	return 0.0;
}