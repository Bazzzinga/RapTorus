#pragma once

#include "Elastic2DFEM.h"

class QuadraticElastic2DFEM : public Elastic2DFEM
{
private:

	void calcMatrixes(FEMElement& FEMelem, const Eigen::Matrix3d& D, std::vector<Eigen::Triplet<double> >& triplets);

	Matrix3Xd calcB6(VectorXd x1, VectorXd x2, double s, double t);

	Matrix3Xd calcB8(VectorXd x1, VectorXd x2, double s, double t);

	double calcElementArea(int elemID);

public:
	QuadraticElastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp);
	~QuadraticElastic2DFEM();
};