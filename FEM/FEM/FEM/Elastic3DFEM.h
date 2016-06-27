#pragma once

#include "FEM.h"

class Elastic3DFEM : public FEM
{
private:
	int formFunctionDerivCoeff[8][12] = {	{-1, 0, -1, -1, -1, -1, 0, -1, -1, -1, -1, 0},
											{ 1, 0, -1, -1, -1,  1, 0, -1, -1,  1, -1, 0},
											{ 1, 0,  1, -1,  1,  1, 0, -1, -1,  1,  1, 0},
											{-1, 0,  1, -1,  1, -1, 0, -1, -1, -1,  1, 0},
											{-1, 0, -1,  1, -1, -1, 0,  1,  1, -1, -1, 0},
											{ 1, 0, -1,  1, -1,  1, 0,  1,  1,  1, -1, 0},
											{ 1, 0,  1,  1,  1,  1, 0,  1,  1,  1,  1, 0},
											{-1, 0,  1,  1,  1, -1, 0,  1,  1, -1,  1, 0} };

protected:

	Eigen::MatrixXd elasticMatrix;

	Eigen::Matrix3d Jacobian8(VectorXd x1, VectorXd x2, VectorXd x3, double s, double t, double u);

	Eigen::Matrix4d Jacobian4(VectorXd x1, VectorXd x2, VectorXd x3);

	double formFunctionDeriv(int index, int denom, double s, double t, double u);

	void SetConstraintsFromList(std::vector<_ConstraintsS> constraint);
	void SetLoadsFromList(std::vector<_Load3DS> loadList);

public:

	void Solve(void);

	Elastic3DFEM(DatabaseAgent * _dba, _PhysicalProperetiesS _pProp);
	~Elastic3DFEM();

	void SetConstraints(int NSID, _ConstraintsS::Type type);
	void SetConstraints(std::string NSName, _ConstraintsS::Type type);

	void SetDisplacement(int NSID, Eigen::VectorXd disp);
	void SetDisplacement(std::string NSName, Eigen::VectorXd disp);

	void SetLoads(int NSID, Eigen::VectorXd force);
	void SetLoads(std::string NSName, Eigen::VectorXd force);
};