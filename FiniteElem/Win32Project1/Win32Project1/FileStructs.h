#pragma once

struct _VertexS
{
	unsigned int id;
	float x1;
	float x2;
	float x3;
};

struct _EdgesS
{
	unsigned short material;
	unsigned short type;
	unsigned short cnst;
	unsigned short section;
	unsigned short cs;
	unsigned short life_flag;
	unsigned short reference;
	unsigned short shape;
	unsigned short nodes_num;
	unsigned short n10;
	unsigned int id;

	std::vector<_VertexS> vertexList;

	bool quad; // false - triangle  true - quad
};