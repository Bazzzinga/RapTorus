#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include "FReader.h"
#include "FileStructs.h"

FReader::FReader(const char* fn)
{
	std::ifstream f(fn);

	std::string line;

	int lineNum = 0;
	//char delim;
	bool edges = false;

	this->err = 0;
	
	/*
		��������� �����:
		- �������������� ����������
		- ������, ������������ � NBLOCK - ������ ����� � ����������� �� �����
			��������� ������� - ������ ����������� �������
			(XiY,ZeU.W)
			��� X - ����� ��������, ������� Y ����� ������� � ����������� �����.
			Z - ����� �������� � ������� � ��������� ������ (������ 3 - ���������� ����, ��������� 3 - ���� ��������)
			���� �������� ������, ��� Z ��������, �� ��� ��������� - ����
			U - ������ �������
			W - ���������� ����� ����� ������� (��������)
			����� ����� ��������� �������� N,R5.3,LOC,  -1{,}
			
		- ������, ������������ � EBLOCK - ������ ����� � ����������� �� ���������
			��������� ������� - ������ ����������� �������
			(XiY) - ���������� NBLOCK
			� 1 ������ ������� �������� �� ����� X ��������, ��������� ����������� �� 2 ������

			����� ����� ��������� �������� -1
	*/

	while (std::getline(f, line))
	{
		if (line.find("NBLOCK") == 0)  //������ ����� � ����������� �� �����
		{
			bool end = false;
						
			std::string nb_format_line;
			
			int numField = atoi(line.substr(7,1).c_str());

			if (line.find("SOLID") != std::string::npos)  // ��������� ��� ������
				this->modelType = 1;
			else
				this->modelType = 0;

			if (!std::getline(f, nb_format_line))  //��������� ������ � �������� ������
				{ this->err = 1;return; }

			bool res = checkBlockFormat(nb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");  // ��������� ��� �� � ��� ������
			if (!res) { this->err = 1; return; }
			_rowFormatS rowFormat = parseNodeBlockFormat(nb_format_line);

			do //��������� ��������� ���������� �� ����� �����
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{	
					if (nb_line.c_str()[0] == 'N')
						end = true;
					else
					{
						_VertexS node = this->parseNodeRow(nb_line, rowFormat);
						_VSVertex.push_back(node);
					}
				}
				else { this->err = 1; return; }
			} while (!end);
		}
		
		if (line.find("EBLOCK") == 0)  //������ ����� � ����������� �� ���������
		{
			bool end = false;

			std::string eb_format_line;

			if (!std::getline(f, eb_format_line))  //��������� ������ � �������� ������
			{ this->err = 1; return; }

			bool res = checkBlockFormat(eb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+)[)]");  // ��������� ��� �� � ��� ������
			if (!res) { this->err = 1; return; }
			_rowFormatS rowFormat = parseElementBlockFormat(eb_format_line);

			do //��������� ��������� ���������� �� ��������� �����
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{
					if ( (nb_line.size() == rowFormat.row_width) && (atoi(nb_line.c_str()) == -1 ))
						end = true;
					else
					{
						//_VertexS node = this->parseNodeRow(nb_line, rowFormat);
						//_VSVertex.push_back(node);
					}
				}
				else { this->err = 1; return; }
			} while (!end);
		}
						
	}

	return;
}

FReader::~FReader()
{
	
}


_VertexS FReader::getVertex(int id)
{
	_VertexS v;
	int _size = this->_VSVertex.size();
	
	if (id >= _size)
		v = this->_VSVertex[0]; //����� ������
	else
		this->_VSVertex[id];
	
	return v;
}

bool FReader::checkBlockFormat(std::string str, const char * e_str)
{
	std::regex exp(e_str);
	return std::regex_match(str, exp);
}

_rowFormatS FReader::parseNodeBlockFormat(std::string str)
{
	std::regex exp("[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");

	std::smatch match;
	std::string::const_iterator pos = str.cbegin();

	std::string temp;
	_rowFormatS res;

	std::regex_search(pos, str.cend(), match, exp);

	temp = match.str(1);
	res.row_num = atoi(temp.c_str());

	temp = match.str(2);
	res.row_width = atoi(temp.c_str());

	temp = match.str(3);
	res.frow_num = atoi(temp.c_str());

	temp = match.str(4);
	res.frow_width = atoi(temp.c_str());

	temp = match.str(5);
	res.precision = atoi(temp.c_str());

	return res;
}

_VertexS FReader::parseNodeRow(std::string str, _rowFormatS rf)
{
	_VertexS res;
	int pos = 0;
	std::string temp;

	temp = str.substr(pos, rf.row_width); //������ ������� - id ��������
	res.node_id = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos > (int)str.size()) { this->err = 1; return res; }  // �������� ������������ ���������� ������

	if (this->modelType == 1)  // ���� SOLID, �� ���� ��� 2 ������� ����� ��������� ���������
	{
		temp = str.substr(pos, rf.row_width);
		res.solid_entity = atoi(temp.c_str());
		pos += rf.row_width;
		if (pos > (int)str.size()) { this->err = 1; return res; }

		temp = str.substr(pos, rf.row_width);
		res.solid_line_location = atoi(temp.c_str());
		pos += rf.row_width;
		if (pos > (int)str.size()) { this->err = 1; return res; }
	}

	for (int i = 0; i < 3; ++i)  // ������� 3 ����������
		if (pos + rf.frow_width <= (int)str.size()) // ���� ��� ����
		{
			temp = str.substr(pos, rf.frow_width);
			res.x[i] = atof(temp.c_str());
			pos += rf.frow_width;
			if (pos >(int)str.size()) { this->err = 1; return res; }
		}
	if (modelType == 1)
	{
		for (int j = 0; j < 3; ++j) // ������� 3 ����
			if (pos + rf.frow_width < (int)str.size()) // ���� ��� ����
			{
				temp = str.substr(pos, rf.frow_width);
				res.a[j] = atof(temp.c_str());
				pos += rf.frow_width;
				if (pos >(int)str.size()) { this->err = 1; return res; }
			}
	}
	return res;
}

_rowFormatS FReader::parseElementBlockFormat(std::string str)
{
	std::regex exp("[(]([[:digit:]]+)i([[:digit:]]+)[)]");

	std::smatch match;
	std::string::const_iterator pos = str.cbegin();

	std::string temp;
	_rowFormatS res;

	std::regex_search(pos, str.cend(), match, exp);

	temp = match.str(1);
	res.row_num = atoi(temp.c_str());

	temp = match.str(2);
	res.row_width = atoi(temp.c_str());

	return res;
}

_EdgesS FReader::parseElementRow(std::string str, _rowFormatS rf)
{
	_EdgesS res;

	return res;
}