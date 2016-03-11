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
		//std::istringstream iss(line);
		
		if (line.find("NBLOCK") == 0)  //начало блока с информацией об узлах
		{
			bool end = false;
						
			std::string nb_format_line;
			int row_num;
			int row_width;
			int frow_num;
			int frow_width;
			int precision;

			if (!std::getline(f, nb_format_line))  //извлекаем строку с форматом данных
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

			do //считываем построчно информацию об узлах сетки
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
		
		if (line.find("EBLOCK") == 0)  //начало блока с информацией об элементах
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
		v = this->_VSVertex[0]; //можно менять
	else
		v = this->_VSVertex[id];
	return v;
}