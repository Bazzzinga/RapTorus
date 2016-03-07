#pragma once
#include <vector>
#include "FileStructs.h"

class FReader
{
private:
	std::vector<_VertexS> _VSVector;
	std::vector<_EdgesS> _VSEdges;
public:
	FReader(const char* fn);
	~FReader();
};