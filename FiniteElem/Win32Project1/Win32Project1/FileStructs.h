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
	unsigned short n1;
	unsigned short n2;
	unsigned short n3;
	unsigned short n4;
	unsigned short n5;
	unsigned short n6;
	unsigned short n7;
	unsigned short n8;
	unsigned short n9;
	unsigned short n10;
	unsigned int id;
	unsigned int v1_id;
	unsigned int v2_id;
	unsigned int v3_id;
	unsigned int v4_id;
	bool quad; // false - triangle  true - quad
};