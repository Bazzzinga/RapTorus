#pragma once

struct _VertexS
{
	unsigned int node_id;
	int solid_entity;
	int solid_line_location;

	double x[3] = { 0, 0, 0 };
	double a[3] = { 0, 0, 0 };
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

struct _rowFormatS
{
	int row_num = 0;
	int row_width = 0;
	int frow_num = 0;
	int frow_width = 0;
	int precision = 0;
};