#pragma once

#include "Database/FileStructs.h"
#include "Database/DatabaseAgent.h"
#include "Database/DatabaseQueries.h"
#include "Eigen/Sparse"
#include "Eigen/Dense"
#include "FEMElement.h"

#define FEM_ERROR_CODE_NO_ERR		0
#define FEM_ERROR_CODE_CANT_INVERSE 1

class FEM
{
private:

	static int databaseSelectNodesCallback(void *data, int argc, char **argv, char **azColName);
	static int databaseSelectElemsCallback(void *data, int argc, char **argv, char **azColName);
	static int databaseSelectElemTypesCallback(void *data, int argc, char **argv, char **azColName);
	static int databaseSelectNSCallback(void *data, int argc, char **argv, char **azColName);
		
protected:
	
	Eigen::SparseMatrix <double> StiffnessMatrix;

	std::vector<_ConstraintsS> constraintList;

	Eigen::VectorXd loads;

	Eigen::VectorXd rightSide;

	Eigen::VectorXd initialDisplacement;

	Eigen::VectorXd displacements;

	DatabaseAgent * dba;

	std::vector<_NodeS> NodeList;
	std::vector<_ElementsS> ElementList;
	std::vector<_ElementTypeS> ElementTypes;
	std::vector<_NamedSetS> NamedSets;

	std::vector<FEMElement> FEMElementList;
			
	_PhysicalProperetiesS pProp;
		
	int errorCode = FEM_ERROR_CODE_NO_ERR;

	int getNodeIndex(unsigned int nodeID);

	int getElemIndex(unsigned int elemID);

	void PushDisplacementsToDB(void);

	void PushSigmaMisesToDB(bool _3d);

public:	

	FEM(DatabaseAgent * _dba);
	~FEM();
};