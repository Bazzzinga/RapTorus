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
		//std::istringstream iss(line);
		
		if (line.find("NBLOCK") == 0)  //������ ����� � ����������� �� �����
		{
			bool end = false;
						
			std::string nb_format_line;
			int row_num;
			int row_width;
			int frow_num;
			int frow_width;
			int precision;

			if (!std::getline(f, nb_format_line))  //��������� ������ � �������� ������
			{
				this->err = 1;
				return;
			}

			bool res;

			std::string temp;

			std::regex exp("[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");
			std::smatch match;
			std::string::const_iterator pos = nb_format_line.cbegin();

			res = std::regex_search(pos, nb_format_line.cend(), match, exp);

			temp = match.str(1);
			row_num = atoi(temp.c_str());

			temp = match.str(2);
			row_width = atoi(temp.c_str());

			temp = match.str(3);
			frow_num = atoi(temp.c_str());

			temp = match.str(4);
			frow_width = atoi(temp.c_str());

			temp = match.str(5);
			precision = atoi(temp.c_str());

			if (!res)
			{
				this->err = 1;
				return;
			}

			do //��������� ��������� ���������� �� ����� �����
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{
					std::string temp;
					
					
				}
				else
				{
					this->err = 1;
					return;
				}
			} while (!end);
		}
		
		if (line.find("EBLOCK") == 0)  //������ ����� � ����������� �� ���������
		{

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
		v = this->_VSVertex[id];
	return v;
}