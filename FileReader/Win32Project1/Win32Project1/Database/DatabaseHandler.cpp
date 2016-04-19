/**
* @file DatabaseHandler.cpp
*/

#include <iostream>
#include "sqlite\sqlite3.h"
#include "DatabaseHandler.h"

/**
* Class destructor.
*/
DatabaseHandler::~DatabaseHandler()
{
	/**
	* Close database connection.
	*/
	int sqlclose_res = 1;

	/**
	* Try closing until it's OK.
	*/
	do
	{
		sqlclose_res = sqlite3_close_v2(this->dbh);
	} while (sqlclose_res != SQLITE_OK);
}

/**
* Initializing function.
* @param[in] dbname a path to SQLite3 database file.
*/
void DatabaseHandler::Init(const char* dbname)
{	
	/**
	* Opening database connection with native sqlite3 library function.
	*/
	int open_res = sqlite3_open_v2(dbname, &this->dbh, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (open_res != 0)
	{
		err = std::string("Can't connect");
		return;
	}	
}

/**
* SQL query execution function.
* @param[in] query string with SQL query.
* @param[in] callback callback function to hadle results of SQL query.
* @param[out] data pointer to variable to hold the expected results of callback function.
*/
void DatabaseHandler::execQuery(std::string query, int(*callback)(void*, int, char**, char**), void* data)
{
	/**
	* a variable to hold result of sqlite3_exec.
	*/
	int sqlres;

	/**
	* Executing SQL query with native sqlite3 library function.
	*/
	sqlres = sqlite3_exec(this->dbh, query.c_str(), callback, data, &sqlerr);

	/**
	* Handling error if occurs.
	*/
	if (sqlres != 0)
	{
		std::cout << sqlerr << "\n";
		sqlite3_free(sqlerr);
	}
}

/**
* Error extracting function.
* @return string with extracted error. Empty string if there is no error.
*/
std::string DatabaseHandler::getError(void)
{
	return this->err;
}