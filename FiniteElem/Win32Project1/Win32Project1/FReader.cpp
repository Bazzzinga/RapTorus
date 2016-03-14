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
		Структура файла:
		- неиспользуемая информация
		- Строка, начинающаяся с NBLOCK - начало блока с информацией об узлах
			следующая строчка - формат последующей таблицы
			(XiY,ZeU.W)
			где X - число столбцов, шириной Y слева таблицы с параметрами узлов.
			Z - число столбцов с числами с плавающей точкой (первые 3 - координаты узла, последние 3 - углы поворота)
			Если выписано меньше, чем Z столбцов, то все остальные - нули
			U - ширина столбца
			W - количество чисел после запятой (точность)
			Конец блока обозначен командой N,R5.3,LOC,  -1{,}
			
		- Строка, начинающаяся с EBLOCK - начало блока с информацией об элементах
			следующая строчка - формат последующей таблицы
			(XiY) - аналогично NBLOCK
			В 1 строке каждого элемента не более X столбцов, остальные переносятся на 2 строку

			Конец блока обозначен командой -1
	*/

	while (std::getline(f, line))
	{
		if (line.find("NBLOCK") == 0)  //начало блока с информацией об узлах
		{
			bool end = false;
						
			std::string nb_format_line;
			
			int numField = atoi(line.substr(7,1).c_str());

			if (line.find("SOLID") != std::string::npos)  // извлекаем тип модели
				this->modelType = 1;
			else
				this->modelType = 0;

			if (!std::getline(f, nb_format_line))  //извлекаем строку с форматом данных
				{ this->err = 1;return; }

			bool res = checkBlockFormat(nb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");  // проверяем нет ли в ней ошибок
			if (!res) { this->err = 1; return; }
			_rowFormatS rowFormat = parseNodeBlockFormat(nb_format_line);

			do //считываем построчно информацию об узлах сетки
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
		
		if (line.find("EBLOCK") == 0)  //начало блока с информацией об элементах
		{
			bool end = false;

			std::string eb_format_line;

			if (!std::getline(f, eb_format_line))  //извлекаем строку с форматом данных
			{ this->err = 1; return; }

			bool res = checkBlockFormat(eb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+)[)]");  // проверяем нет ли в ней ошибок
			if (!res) { this->err = 1; return; }
			_rowFormatS rowFormat = parseElementBlockFormat(eb_format_line);

			do //считываем построчно информацию об элементах сетки
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{
					if ( (nb_line.size() == rowFormat.row_width) && (atoi(nb_line.c_str()) == -1 ))
						end = true;
					else
					{
						_EdgesS node = this->parseElementRow(nb_line, rowFormat, &f);
						_VSEdges.push_back(node);
					}
				}
				else { this->err = 1; return; }
			} while (!end);
		}
						
	}

	f.close();

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
		v = this->_VSVertex[0]; //можно менять
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

	temp = str.substr(pos, rf.row_width); //первый столбец - id элемента
	res.node_id = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos > (int)str.size()) { this->err = 1; return res; }  // проверка правильности переданной строки

	if (this->modelType == 1)  // если SOLID, то есть еще 2 столбца перед столбцами координат
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

	for (int i = 0; i < 3; ++i)  // достаем 3 коориднаты
		if (pos + rf.frow_width <= (int)str.size()) // если они есть
		{
			temp = str.substr(pos, rf.frow_width);
			res.x[i] = atof(temp.c_str());
			pos += rf.frow_width;
			if (pos >(int)str.size()) { this->err = 1; return res; }
		}
	if (modelType == 1)
	{
		for (int j = 0; j < 3; ++j) // достаем 3 угла
			if (pos + rf.frow_width < (int)str.size()) // если они есть
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

_EdgesS FReader::parseElementRow(std::string str, _rowFormatS rf, std::ifstream * f)
{
	_EdgesS res;
	std::string second_line;
	std::string temp;
	
	int pos = 0;

	temp = str.substr(pos, rf.row_width);  //получаем id материала
	res.material = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем тип элемента
	res.type = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем real constant number
	res.cnst = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем id секции
	res.section = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем id системы координат
	res.cs = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем флаг рождения/смерти
	res.life_flag = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }
	
	temp = str.substr(pos, rf.row_width);  //получаем номер соответствующей solid-модели
	res.reference = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем флаг формы элемента
	res.shape = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем число узлов в элементе
	res.nodes_num = atoi(temp.c_str());
	pos += rf.row_width;

		
	pos += rf.row_width;  // 10-ый столбец не используется
	if (pos >(int)str.size()) { this->err = 1; return res; }

	temp = str.substr(pos, rf.row_width);  //получаем id элемента
	res.id = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = 1; return res; }

	int col_count = (res.nodes_num < EDGE_NODES_COL_COUNT)? res.nodes_num: EDGE_NODES_COL_COUNT;

	for (int i = 0; i < col_count; ++i)
	{
		temp = str.substr(pos, rf.row_width);  //получаем id элемента		
		res.vertexIDList.push_back(atoi(temp.c_str()));
		pos += rf.row_width;
		if (pos >(int)str.size()) { this->err = 1; return res; }
	}

	if (res.nodes_num > EDGE_NODES_COL_COUNT)  // если список узлов не помещается на 1 строчке
	{
		std::getline(*f, second_line);

		int second_line_pos = 0;

		for (int j = 0; j < res.nodes_num - EDGE_NODES_COL_COUNT; ++j)
		{
			temp = second_line.substr(second_line_pos, rf.row_width);  //получаем id элемента		
			res.vertexIDList.push_back(atoi(temp.c_str()));
			second_line_pos += rf.row_width;
			if (second_line_pos >(int)str.size()) { this->err = 1; return res; }
		}
	}

	return res;
}