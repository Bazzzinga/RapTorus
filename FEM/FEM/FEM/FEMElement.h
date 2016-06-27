#pragma once

#include <vector>
#include "Eigen/Sparse"
#include "Eigen/Dense"
#include "Database/FileStructs.h"

typedef Eigen::Matrix<double, 3, Eigen::Dynamic> Matrix3Xd;
typedef Eigen::Matrix<double, Eigen::Dynamic, 3> MatrixX3d;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixXXd;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> VectorXd;


class FEMElement
{
public:
	int elemID;

	std::vector<int> nodeIDs;

	int nodeCount;

	MatrixXXd B;

	double S;

	double sigma_mises;

	FEMElement(_ElementsS elem);
	~FEMElement();
};