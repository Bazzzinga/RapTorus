#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "FReader.h"
#include "FileStructs.h"

FReader::FReader(const char* fn)
{
	std::ifstream f(fn);

	std::string line;

	int _dimm;
	int lineNum = 0;
	int magic_n1, magic_n2;
	char delim;
	bool edges = false;
	
	while (std::getline(f, line))
	{
		std::istringstream iss(line);

		if (lineNum == 0 || lineNum == 2)
		{
			lineNum++;
			continue;
		}

		if (lineNum == 1)
		{
			iss >> _dimm;
			lineNum++;
			continue;
		}

		if (!edges)
		{
			_VertexS v;
				
			if (!(iss >> v.id >> v.x1 >> v.x2 >> v.x3))
			{
				iss = std::istringstream(line);
				iss >> magic_n1 >> delim >> magic_n2;
				edges = true;
			}
			if(!edges)
				_VSVector.push_back(v);
		}
		else
		{
			_EdgesS e;

			if (!(iss >> e.n1 >> e.n2 >> e.n3 >> e.n4 >> e.n5 >> e.n6 >> e.n7 >> e.n8 >> e.n9 >> e.n10 >> e.id >> e.v1_id >> e.v2_id >> e.v3_id >> e.v4_id))
			{
				e.v4_id = 0;
				e.quad = false;
			}
			else
				e.quad = true;

			_VSEdges.push_back(e);
		}

		lineNum++;
	}

	return;
}

FReader::~FReader()
{

}