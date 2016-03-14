#pragma once
#include <vector>
#include "FileStructs.h"

class FReader
{
private:
	int modelType;  // 0 - SHELL   1 - SOLID 
	int err;
	std::vector<_VertexS> _VSVertex;
	std::vector<_EdgesS> _VSEdges;

	bool checkBlockFormat(std::string str, const char * e_str);

	_rowFormatS parseNodeBlockFormat(std::string str);
	_VertexS parseNodeRow(std::string str, _rowFormatS rf);

	_rowFormatS parseElementBlockFormat(std::string str);
	_EdgesS parseElementRow(std::string str, _rowFormatS rf, std::ifstream * f);


public:
	FReader(const char* fn);
	~FReader();

	_VertexS getVertex(int id);
};