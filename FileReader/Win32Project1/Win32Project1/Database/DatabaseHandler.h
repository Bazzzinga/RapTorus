/**
* @file DatabaseHandler.h
*/

#pragma once

#include <string>
#include "sqlite\sqlite3.h"
#include "DatabaseQueries.h"

/**
 * DatabaseHandler class.
 * Holds the handle of SQLite3 database connection.
 */
class DatabaseHandler
{
private:
	sqlite3 * dbh = 0;		/**< database connection handle pointer.	*/

	std::string err = "";	/**< DatabaseHandler error description string.	*/

	char* sqlerr;			/**< SQLite error.	*/

public:
	/**
	* Class constructor.
	*/
	DatabaseHandler() {};

	/**
	* Class destructor.
	*/
	~DatabaseHandler();

	/**
	* Initializing function.
	* @param[in] dbname a path to SQLite3 database file.
	*/
	void Init(const char* dbname);

	/**
	* SQL query execution function.
	* @param[in] query string with SQL query.
	* @param[in] callback callback function to hadle results of SQL query.
	* @param[out] data pointer to variable to hold the expected results of callback function.
	*/
	void execQuery(std::string query, int(*callback)(void*, int, char**, char**), void* data);

	/**
	* Error extracting function.
	* @return string with extracted error. Empty string if there is no error.
	*/
	std::string getError(void);
};