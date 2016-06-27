#pragma once

#include "FEM.h"

class Elastic2DFEM : public FEM
{
protected:
	double elementHeight;

	Eigen::Matrix3d elasticMatrix;

	Eigen::Matrix2d Jacobian4(VectorXd x1, VectorXd x2, double s, double t);

	void SetConstraintsFromList(std::vector<_ConstraintsS> constraint);
	void SetLoadsFromList(std::vector<_Load2DS> loadList);

public:
		
	void Solve(void);

	Elastic2DFEM(DatabaseAgent * _dba, double h, _PhysicalProperetiesS _pProp);
	~Elastic2DFEM();

	void SetConstraints(int NSID, _ConstraintsS::Type type);
	void SetConstraints(std::string NSName, _ConstraintsS::Type type);

	void SetDisplacement(int NSID, Eigen::VectorXd disp);
	void SetDisplacement(std::string NSName, Eigen::VectorXd disp);

	void SetLoads(int NSID, Eigen::VectorXd force);
	void SetLoads(std::string NSName, Eigen::VectorXd force);
};
