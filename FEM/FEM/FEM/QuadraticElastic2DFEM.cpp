#include "QuadraticElastic2DFEM.h"

#include "QuadraticElastic2DFEM.h"

QuadraticElastic2DFEM::QuadraticElastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp) : Elastic2DFEM(_dba, h, _pProp)
{
	std::vector<Eigen::Triplet<double>> triplets;

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
		calcMatrixes(*it, elasticMatrix, triplets);

	StiffnessMatrix.resize(2 * (int)NodeList.size(), 2 * (int)NodeList.size());
	StiffnessMatrix.setFromTriplets(triplets.begin(), triplets.end());
}

QuadraticElastic2DFEM::~QuadraticElastic2DFEM()
{

}

Matrix3Xd QuadraticElastic2DFEM::calcB6(VectorXd x1, VectorXd x2, double s, double t)
{
	Matrix3Xd B;

	double u = 1.0 - s - t;

	B.resize(3, 2 * (int)x1.size());
	
	Eigen::Matrix3d J, _J;

	_J << 1, 1, 1,
		x1(0), x1(1), x1(2),
		x2(0), x2(1), x2(2);

	J = _J.inverse();

	for (int jj = 0; jj < (int)x1.size(); ++jj)
	{
		Eigen::Vector2d xy_dN;

		switch (jj)
		{
		case 0:
			xy_dN(0) = J(0, 1) * (4.0 * s - 1.0);
			xy_dN(1) = J(0, 2) * (4.0 * s - 1.0);
			break;
		case 1:
			xy_dN(0) = J(1, 1) * (4.0 * t - 1.0);
			xy_dN(1) = J(1, 2) * (4.0 * t - 1.0);
			break;
		case 2:
			xy_dN(0) = J(2, 1) * (4.0 * u - 1.0);
			xy_dN(1) = J(2, 2) * (4.0 * u - 1.0);
			break;
		case 3:
			xy_dN(0) = 4.0 * (s * J(1, 1) + t * J(0, 1));
			xy_dN(1) = 4.0 * (s * J(1, 2) + t * J(0, 2));
			break;
		case 4:
			xy_dN(0) = 4.0 * (t * J(2, 1) + u * J(1, 1));
			xy_dN(1) = 4.0 * (t * J(2, 2) + u * J(1, 2));
			break;
		case 5:
			xy_dN(0) = 4.0 * (s * J(2, 1) + u * J(0, 1));
			xy_dN(1) = 4.0 * (s * J(2, 2) + u * J(0, 2));
			break;
		}
		
		B(0, 2 * jj + 0) = xy_dN(0);
		B(0, 2 * jj + 1) = 0.0;
		B(1, 2 * jj + 0) = 0.0;
		B(1, 2 * jj + 1) = xy_dN(1);
		B(2, 2 * jj + 0) = xy_dN(1);
		B(2, 2 * jj + 1) = xy_dN(0);
	}

	return B;
}


Matrix3Xd QuadraticElastic2DFEM::calcB8(VectorXd x1, VectorXd x2, double s, double t)
{
	Matrix3Xd B;

	B.resize(3, 2 * (int)x1.size());

	for (int jj = 0; jj < (int)x1.size(); ++jj)
	{
		Eigen::Vector2d xy_dN, st_dN;

		switch (jj)
		{
		case 0:
			st_dN(0) = (1.0 - t) * (2.0 * s + t) / 4.0;
			st_dN(1) = (1.0 - s) * (2.0 * t + s) / 4.0;
			break;
		case 1:
			st_dN(0) = (1.0 - t) * (2.0 * s - t) / 4.0;
			st_dN(1) = (1.0 + s) * (2.0 * t - s) / 4.0;
			break;
		case 2:
			st_dN(0) = (1.0 + t) * (2.0 * s + t) / 4.0;
			st_dN(1) = (1.0 + s) * (2.0 * t + s) / 4.0;
			break;
		case 3:
			st_dN(0) = (1.0 + t) * (2.0 * s - t) / 4.0;
			st_dN(1) = (1.0 - s) * (2.0 * t - s) / 4.0;
			break;
		case 4:
			st_dN(0) = s * (t - 1.0);
			st_dN(1) = (s * s - 1.0) / 2.0;
			break;
		case 5:
			st_dN(0) = (1.0 - t * t) / 2.0;
			st_dN(1) = -t * (1.0 + s);
			break;
		case 6:
			st_dN(0) = -s * (1.0 + t);
			st_dN(1) = (1.0 - s * s) / 2.0;
			break;
		case 7:
			st_dN(0) = (t * t - 1.0) / 2.0;
			st_dN(1) = t * (s - 1.0);
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

void QuadraticElastic2DFEM::calcMatrixes(FEMElement& FEMelem, const Eigen::Matrix3d& D, std::vector<Eigen::Triplet<double> >& triplets)
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

	if (FEMelem.nodeCount == 6)
	{
		double gaussPointX[3] = { 1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0 };
		double gaussPointY[3] = { 1.0 / 6.0, 1.0 / 6.0, 2.0 / 3.0 };
		double gaussWeight = 1.0 / 6.0;

		for (int i = 0; i < 3; ++i)
		{
			Eigen::Matrix3Xd BB = calcB6(Nodex1, Nodex2, gaussPointX[i], gaussPointY[i]);

			K += BB.transpose() * D * BB;
		}

		FEMelem.B = calcB6(Nodex1, Nodex2, 1.0 / 3.0, 1.0 / 3.0);

		K *= elementHeight * gaussWeight * FEMelem.S / 2.0;
	}
	else if (FEMelem.nodeCount == 8)
	{
		double gaussPoint = 1 / sqrt(3);

		for (int i = -1; i <= 1; i += 2)
			for (int j = -1; j <= 1; j += 2)
			{
				Eigen::Matrix3Xd BB = calcB8(Nodex1, Nodex2, gaussPoint * i, gaussPoint * j);

				K += 4.0 * Jacobian4(Nodex1, Nodex2, gaussPoint * i, gaussPoint * j).determinant() * BB.transpose() * D * BB;
			}

		FEMelem.B = calcB8(Nodex1, Nodex2, 0.0, 0.0);

		K *= elementHeight;
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

double QuadraticElastic2DFEM::calcElementArea(int elemID)
{
	VectorXd Nodex1;
	VectorXd Nodex2;

	double area1 = 0;
	double area2 = 0;

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
		if (it->elemID == elemID)
		{
			int nodeCount = it->nodeCount / 2;
			Nodex1.resize(nodeCount);
			Nodex2.resize(nodeCount);


			for (int k = 0; k < nodeCount; ++k)
			{
				int index = getNodeIndex(it->nodeIDs[k]);

				Nodex1(k) = NodeList[index].x[0];
				Nodex2(k) = NodeList[index].x[1];
			}

			for (int ii = 0; ii < nodeCount; ++ii)
			{
				int kk = ii + 1;
				if (kk == nodeCount)
					kk = 0;

				area1 += Nodex1[ii] * Nodex2[kk];
				area2 += Nodex1[kk] * Nodex2[ii];
			}

			return (area1 - area2) / 2.0;
		}
	return 0.0;
}