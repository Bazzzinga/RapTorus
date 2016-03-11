#pragma once
#include <vector>
#include "FileStructs.h"

class FReader
{
private:
	int modelType;
	int _dimm;
	int err;
	std::vector<_VertexS> _VSVertex;
	std::vector<_EdgesS> _VSEdges;
	//int magic_n1, magic_n2;
public:
	FReader(const char* fn);
	~FReader();

	_VertexS getVertex(int id);
};