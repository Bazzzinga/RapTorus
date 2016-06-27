#pragma once

#include "Elastic2DFEM.h"

class LinearElastic2DFEM : public Elastic2DFEM
{
private:
	
	void calcMatrixes(FEMElement& FEMelem, const Eigen::Matrix3d& D, std::vector<Eigen::Triplet<double> >& triplets);

	Matrix3Xd calcB4(VectorXd x1, VectorXd x2, double s, double t);

	Matrix3Xd calcB3(VectorXd x1, VectorXd x2);
	
	double calcElementArea(int elemID);

public:	
	LinearElastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp);
	~LinearElastic2DFEM();
};