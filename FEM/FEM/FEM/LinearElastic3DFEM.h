#pragma once

#include "Elastic3DFEM.h"

class LinearElastic3DFEM : public Elastic3DFEM
{
private:

	void calcMatrixes(FEMElement& FEMelem, const Eigen::MatrixXd& D, std::vector<Eigen::Triplet<double> >& triplets);

	MatrixXXd calcB8(VectorXd x1, VectorXd x2, VectorXd x3, double s, double t, double u);

	MatrixXXd calcB4(VectorXd x1, VectorXd x2,VectorXd x3);

public:
	LinearElastic3DFEM(DatabaseAgent * _dba, _PhysicalProperetiesS _pProp);
	~LinearElastic3DFEM();
};