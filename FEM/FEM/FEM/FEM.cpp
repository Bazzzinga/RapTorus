#include <iostream>
#include <fstream>
#include "FEM.h"

int FEM::databaseSelectNodesCallback(void *data, int argc, char **argv, char **azColName)
{
	std::vector<_NodeS> * list = (std::vector<_NodeS>*)data;

	_NodeS node;

	node.node_id	= atoi(argv[0]);
	node.x[0]		= atof(argv[1]);
	node.x[1]		= atof(argv[2]);
	node.x[2]		= atof(argv[3]);

	node.a[0]		= atof(argv[4]);
	node.a[1]		= atof(argv[5]);
	node.a[2]		= atof(argv[6]);

	node.solid_entity = atoi(argv[7]);

	node.solid_line_location = atoi(argv[8]);

	list->push_back(node);

	return 0;
}

int FEM::databaseSelectElemsCallback(void *data, int argc, char **argv, char **azColName)
{
	std::vector<_ElementsS> * list = (std::vector<_ElementsS>*)data;

	_ElementsS elem;

	elem.id = atoi(argv[0]);

	if (((int)list->size() == 0) || ((*list)[list->size() - 1].id != elem.id))
	{
		elem.material = atoi(argv[1]);
		elem.type = atoi(argv[2]);
		elem.cnst = atoi(argv[3]);
		elem.section = atoi(argv[4]);
		elem.cs = atoi(argv[5]);
		elem.life_flag = atoi(argv[6]);
		elem.reference = atoi(argv[7]);
		elem.shape = atoi(argv[8]);
		elem.nodes_num = atoi(argv[9]);	

		elem.nodeIDList.push_back(atoi(argv[10]));

		list->push_back(elem);
	}
	else
		(*list)[list->size() - 1].nodeIDList.push_back(atoi(argv[10]));
	
	return 0;
}

int FEM::databaseSelectElemTypesCallback(void *data, int argc, char **argv, char **azColName)
{
	std::vector<_ElementTypeS> * list = (std::vector<_ElementTypeS>*)data;

	_ElementTypeS temp;

	temp.ID = atoi(argv[0]);
	temp.type = atoi(argv[1]);

	list->push_back(temp);

	return 0;
}

int FEM::databaseSelectNSCallback(void *data, int argc, char **argv, char **azColName)
{
	std::vector<_NamedSetS> * list = (std::vector<_NamedSetS>*)data;

	int setid = atoi(argv[0]);

	if ( ((int)list->size() == 0) || ((*list)[(int)list->size() - 1].SetID != setid) )
	{
		_NamedSetS temp;

		temp.SetID = setid;
		temp.type = atoi(argv[2]);
		temp.itemList.push_back(atoi(argv[1]));
		temp.name = std::string(argv[3]);

		list->push_back(temp);

		return 0;
	}
	
	(*list)[(int)list->size() - 1].itemList.push_back(atoi(argv[1]));

	return 0;
}

FEM::FEM(DatabaseAgent * _dba)
{
	this->dba = _dba;

	std::string sqlQueryNodes = "SELECT NodeID, x, y, z, a1, a2, a3, solidEntity, solidLineLocation FROM Nodes";
	this->dba->exec(sqlQueryNodes, databaseSelectNodesCallback, &NodeList);

	std::string sqlQueryElems = "SELECT e.ElementID, e.material, e.type, e.cnst, e.section, e.cs, e.life_flag, e.reference, e.shape, e.nodes_sum, en.NodeID FROM ElementNodes en JOIN Elements e ON e.ElementID = en.ElementID";
	this->dba->exec(sqlQueryElems, databaseSelectElemsCallback, &ElementList);

	for (std::vector<_ElementsS>::iterator it = ElementList.begin(); it != ElementList.end(); ++it)
	{		
		FEMElement temp(*it);
		FEMElementList.push_back(temp);
	}

	std::string sqlQueryElemTypes = "SELECT TypeID, TypeName FROM ElementTypes";
	this->dba->exec(sqlQueryElemTypes, databaseSelectElemTypesCallback, &ElementTypes);

	std::string sqlQueryNS = "SELECT i.SetID, i.ItemID, s.Type, s.Name FROM NamedSetItems i JOIN NamedSets s ON i.SetID = s.SetID";
	this->dba->exec(sqlQueryNS, databaseSelectNSCallback, &NamedSets);
}

FEM::~FEM()
{

}


int FEM::getNodeIndex(unsigned int nodeID)
{
	int l = 0;
	int r = this->NodeList.size() - 1;

	while (l <= r)
	{
		int m = (l + r) / 2;
		if (NodeList[m].node_id == nodeID) return m;
		if (NodeList[m].node_id < nodeID) l = m + 1;
		else r = m - 1;
	}

	return -1;

}

int FEM::getElemIndex(unsigned int elemID)
{
	int l = 0;
	int r = this->ElementList.size() - 1;

	while (l <= r)
	{
		int m = (l + r) / 2;
		if (ElementList[m].id == elemID) return m;
		if (ElementList[m].id  < elemID) l = m + 1;
		else r = m - 1;
	}

	return -1;
}

void FEM::PushDisplacementsToDB(void)
{
	dba->exec(c_DisplacementTableCreationQuery, NULL, NULL);

	for (int i = 0; i < (int)NodeList.size(); ++i)
	{
		char * sqlQueryD;
		
		if((int)displacements.size() == 2 * (int)NodeList.size())
			sqlQueryD = sqlite3_mprintf(c_insertDisplacement2D.c_str(), NodeList[i].node_id, displacements(2 * i), displacements(2 * i + 1));
		else
			sqlQueryD = sqlite3_mprintf(c_insertDisplacement3D.c_str(), NodeList[i].node_id, displacements(3 * i), displacements(3 * i + 1), displacements(3 * i + 2));

		dba->execBuffered(std::string(sqlQueryD));

		sqlite3_free(sqlQueryD);
	}

	dba->freeBuffer();
}

void FEM::PushSigmaMisesToDB(bool _3d)
{
	if(_3d)
		dba->exec(c_Stress3DTableCreationQuery, NULL, NULL);
	else
		dba->exec(c_StressTableCreationQuery, NULL, NULL);

	for (int i = 0; i < (int)FEMElementList.size(); ++i)
	{
		char * sqlQueryS;
		if (_3d)
			sqlQueryS = sqlite3_mprintf(c_insertSigma3DMises.c_str(), ElementList[i].id, FEMElementList[i].sigma_mises);
		else
			sqlQueryS = sqlite3_mprintf(c_insertSigmaMises.c_str(), ElementList[i].id, FEMElementList[i].sigma_mises);

		dba->execBuffered(std::string(sqlQueryS));

		sqlite3_free(sqlQueryS);
	}

	dba->freeBuffer();
}