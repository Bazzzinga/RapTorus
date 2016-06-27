#include "LinearElastic3DFEM.h"

LinearElastic3DFEM::LinearElastic3DFEM(DatabaseAgent * _dba, _PhysicalProperetiesS _pProp) : Elastic3DFEM(_dba, _pProp)
{
	std::vector<Eigen::Triplet<double>> triplets;

	for (std::vector<FEMElement>::iterator it = FEMElementList.begin(); it != FEMElementList.end(); ++it)
		calcMatrixes(*it, elasticMatrix, triplets);

	StiffnessMatrix.resize(3 * (int)NodeList.size(), 3 * (int)NodeList.size());

	StiffnessMatrix.setFromTriplets(triplets.begin(), triplets.end());
}

LinearElastic3DFEM::~LinearElastic3DFEM()
{

}

MatrixXXd LinearElastic3DFEM::calcB8(VectorXd x1, VectorXd x2, VectorXd x3, double s, double t, double u)
{
	MatrixXXd B;

	B.resize(6, 3 * (int)x1.size());

	for (int i = 0; i < (int)x1.size(); ++i)
	{
		Eigen::Vector3d xyz_dN, stu_dN;

		stu_dN(0) = formFunctionDeriv(i, 0, s, t, u);
		stu_dN(1) = formFunctionDeriv(i, 1, s, t, u);
		stu_dN(2) = formFunctionDeriv(i, 2, s, t, u);

		xyz_dN = Jacobian8(x1, x2, x3, s, t, u).inverse() * stu_dN;

		B(0, 3 * i + 0) = xyz_dN(0);
		B(0, 3 * i + 1) = 0.0;
		B(0, 3 * i + 2) = 0.0;
		B(1, 3 * i + 0) = 0.0;
		B(1, 3 * i + 1) = xyz_dN(1);
		B(1, 3 * i + 2) = 0.0;
		B(2, 3 * i + 0) = 0.0;
		B(2, 3 * i + 1) = 0.0;
		B(2, 3 * i + 2) = xyz_dN(2);
		B(3, 3 * i + 0) = xyz_dN(1);
		B(3, 3 * i + 1) = xyz_dN(0);
		B(3, 3 * i + 2) = 0.0;
		B(4, 3 * i + 0) = 0.0;
		B(4, 3 * i + 1) = xyz_dN(2);
		B(4, 3 * i + 2) = xyz_dN(1);
		B(5, 3 * i + 0) = xyz_dN(2);
		B(5, 3 * i + 1) = 0.0;
		B(5, 3 * i + 2) = xyz_dN(0);		
	}

	return B;
}

Eigen::Vector3i getJKLindices(int i)
{
	Eigen::Vector3i res;

	int temp = i;

	for (int ii = 0; ii < 3; ++ii)
	{
		++temp;

		if (temp > 4)
			temp = 1;		

		res(ii) = temp;		
	}

	return res;	
}

MatrixXXd LinearElastic3DFEM::calcB4(VectorXd x1, VectorXd x2, VectorXd x3)
{
	MatrixXXd B;

	B.resize(6, 3 * (int)x1.size());

	Eigen::Matrix4d A = Jacobian4(x1, x2, x3);
	Eigen::Matrix4d _A = A.inverse();
	
	for (int i = 0; i < (int)x1.size(); ++i)
	{			
		B(0, 3 * i + 0) = _A(i, 1);
		B(0, 3 * i + 1) = 0.0;
		B(0, 3 * i + 2) = 0.0;
		B(1, 3 * i + 0) = 0.0;
		B(1, 3 * i + 1) = _A(i, 2);
		B(1, 3 * i + 2) = 0.0;
		B(2, 3 * i + 0) = 0.0;
		B(2, 3 * i + 1) = 0.0;
		B(2, 3 * i + 2) = _A(i, 3);
		B(3, 3 * i + 0) = _A(i, 2);
		B(3, 3 * i + 1) = _A(i, 1);
		B(3, 3 * i + 2) = 0.0;
		B(4, 3 * i + 0) = 0.0;
		B(4, 3 * i + 1) = _A(i, 3);
		B(4, 3 * i + 2) = _A(i, 2);
		B(5, 3 * i + 0) = _A(i, 3);
		B(5, 3 * i + 1) = 0.0;
		B(5, 3 * i + 2) = _A(i, 1);		
	}

	return B;
}

void LinearElastic3DFEM::calcMatrixes(FEMElement& FEMelem, const Eigen::MatrixXd& D, std::vector<Eigen::Triplet<double> >& triplets)
{
	VectorXd Nodex1;
	VectorXd Nodex2;
	VectorXd Nodex3;

	std::vector<int> NodeIndexes;

	MatrixXXd K;

	K.resize(3 * FEMelem.nodeCount, 3 * FEMelem.nodeCount);

	K.setZero();

	Nodex1.resize(FEMelem.nodeCount);
	Nodex2.resize(FEMelem.nodeCount);
	Nodex3.resize(FEMelem.nodeCount);

	for (int k = 0; k < FEMelem.nodeCount; ++k)
	{
		int index = getNodeIndex(FEMelem.nodeIDs[k]);
		NodeIndexes.push_back(index);

		Nodex1(k) = NodeList[index].x[0];
		Nodex2(k) = NodeList[index].x[1];
		Nodex3(k) = NodeList[index].x[2];
	}

	if (FEMelem.nodeCount == 4)
	{		
		FEMelem.B = calcB4(Nodex1, Nodex2, Nodex3);

		//S as volume
		FEMelem.S = Jacobian4(Nodex1, Nodex2, Nodex3).transpose().determinant() / 6.0;
				
		K += FEMelem.S * FEMelem.B.transpose() * D * FEMelem.B;
	}
	else if (FEMelem.nodeCount == 8)
	{

		//интегрировать по гауссу
		double gaussPoint = 1 / sqrt(3);
				
		for (int i = -1; i <= 1; i += 2)
			for (int j = -1; j <= 1; j += 2)
				for (int k = -1; k <= 1; k+=2)
			{
				MatrixXXd BB = calcB8(Nodex1, Nodex2, Nodex3, gaussPoint * i, gaussPoint * j, gaussPoint * k);

				K += Jacobian8(Nodex1, Nodex2, Nodex3, gaussPoint * i, gaussPoint * j, gaussPoint * k).determinant() * BB.transpose() * D * BB;
			}

		FEMelem.B = calcB8(Nodex1, Nodex2, Nodex3, 0.0, 0.0, 0.0);				
	}
	
	for (int i = 0; i < FEMelem.nodeCount; ++i)
	{
		for (int j = 0; j < FEMelem.nodeCount; ++j)
		{
			Eigen::Triplet<double> trpl11(3 * NodeIndexes[i] + 0, 3 * NodeIndexes[j] + 0, K(3 * i + 0, 3 * j + 0));
			Eigen::Triplet<double> trpl12(3 * NodeIndexes[i] + 0, 3 * NodeIndexes[j] + 1, K(3 * i + 0, 3 * j + 1));
			Eigen::Triplet<double> trpl13(3 * NodeIndexes[i] + 0, 3 * NodeIndexes[j] + 2, K(3 * i + 0, 3 * j + 2));
			Eigen::Triplet<double> trpl21(3 * NodeIndexes[i] + 1, 3 * NodeIndexes[j] + 0, K(3 * i + 1, 3 * j + 0));
			Eigen::Triplet<double> trpl22(3 * NodeIndexes[i] + 1, 3 * NodeIndexes[j] + 1, K(3 * i + 1, 3 * j + 1));
			Eigen::Triplet<double> trpl23(3 * NodeIndexes[i] + 1, 3 * NodeIndexes[j] + 2, K(3 * i + 1, 3 * j + 2));
			Eigen::Triplet<double> trpl31(3 * NodeIndexes[i] + 2, 3 * NodeIndexes[j] + 0, K(3 * i + 2, 3 * j + 0));
			Eigen::Triplet<double> trpl32(3 * NodeIndexes[i] + 2, 3 * NodeIndexes[j] + 1, K(3 * i + 2, 3 * j + 1));
			Eigen::Triplet<double> trpl33(3 * NodeIndexes[i] + 2, 3 * NodeIndexes[j] + 2, K(3 * i + 2, 3 * j + 2));

			triplets.push_back(trpl11);
			triplets.push_back(trpl12);
			triplets.push_back(trpl13);
			triplets.push_back(trpl21);
			triplets.push_back(trpl22);
			triplets.push_back(trpl23);
			triplets.push_back(trpl31);
			triplets.push_back(trpl32);
			triplets.push_back(trpl33);
		}
	}
}