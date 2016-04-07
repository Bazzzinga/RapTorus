/**
 * @file FileReader.cpp
 */

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include <string>
#include <chrono>
#include <ratio>
#include "FReader.h"
#include "Database\DatabaseAgent.h"

/**
* Main project function. Argument format (can be used in any order):\n
* -f - path to the file to be opened. Required parameter.\n
*-t - desired database file extension. Optional parameter. Default value is "pdb".\n
* -p - desired database path. Optional parameter. Default value is "\".
* @param argc default main function parameter - defines size of argv array.
* @param argv default main function parameter - array of input parameters. 
* @return integer result: 0 - if everything is OK, not 0 if there is a problem.
*/
int main(int argc, char *argv[])
{
	auto timestamp1 = std::chrono::high_resolution_clock::now();
	std::string file;
	std::string fileExt;
	std::string fileName;
	std::string filePath;
	std::string databasePath = DEFAULT_DATABASE_FILE_PATH;
	std::string databaseFileExt = DEFAULT_DATABASE_FILE_TYPE;

	if (argc < 3)
	{
		std::cout << "Not enough parameters. Read documentation!\n\n";
		return 1;
	}

	/**
	 * extracting parameters from command line
	 * argv[0] - path to current .exe file
	 */
	for (int i = 1; i < argc; ++i)
	{
		if (std::string(argv[i]).compare("-f") == 0)
		{
			if (i >= argc - 1)
			{
				std::cout << "No -f parameter. Read documentation!\n\n";
				return 1;
			}
			
			
			file = std::string(argv[i + 1]);
				
			/**
			 * looking for '.' in string to get separated file name and file extention
			 */

			int dotPos = (int)file.find_last_of('.');
				
			if ( (dotPos >= (int)file.size() - 1) || (dotPos < 0) )
			{
				std::cout << "Wrong file name!\n\n";
				return 1;
			}

			fileExt = file.substr(dotPos + 1);

			/**
			 * checking file extention for wrong symbols
			 */

			for (int j = 0; j < (int)fileExt.size(); ++j)
				if ( !( ( (fileExt[j] >= 'a') && (fileExt[j] <= 'z') ) || ( (fileExt[j] >= 'A') && (fileExt[j] <= 'Z') ) ) )
				{
					std::cout << "Wrong file name!\n\n";
					return 1;
				}

			int nameStart = (int)fileName.find('/');

			filePath = file.substr(0, (int)file.size() - (int)fileExt.size() - 1);
			fileName = file.substr(nameStart + 1, (int)file.size() - (int)fileExt.size() - 1);
					
		}
		else if (std::string(argv[i]).compare("-t") == 0)
		{
			if (i >= argc - 1)
				std::cout << "No -t parameter. Using default \"" << DEFAULT_DATABASE_FILE_TYPE << "\" database file type.\n";
			else
			{
				databaseFileExt = std::string(argv[i + 1]);
			}
		}
		else if (std::string(argv[i]).compare("-p") == 0)
		{
			if (i >= argc - 1)
				std::cout << "No -p parameter. Using default \"" << DEFAULT_DATABASE_FILE_PATH << "\" database file type.\n";
			else
			{
				databasePath = std::string(argv[i + 1]);
			}
		}
	}

	/**
	 * Creating database wrapper.
	 */
	DatabaseAgent DBH(databasePath.append(fileName).append(".").append(databaseFileExt).c_str());

	/**
	 * Reading mesh file to database.
	 */
	FReader FR(file.c_str(), &DBH);

	FR_ERROR frErr = FR.getError();
	
	switch (frErr)
	{
	case ERR_NO_ERR:
		break;
	case ERR_OPEN:
		std::cout << "Error opening file!\n";
		break;
	case ERR_WRONG_FILE_FORMAT:
		std::cout << "Wrong input file format!\n";
		break;	
	}

	auto timestamp2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = timestamp2 - timestamp1;

	std::cout << "File " << file.c_str() << " was successfully converted into database " << databasePath << " in " << duration.count() / 1000 << " s";
    return 0;
}

