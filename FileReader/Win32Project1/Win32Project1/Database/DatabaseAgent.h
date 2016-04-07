/**
 * @file DatabaseAgent.h
 */

#pragma once

#include <string>
#include "DatabaseHandler.h"

#define QUERY_BUFFER_MAX_SIZE 1000  /**< Maximum size of query buffer. */

/**
 * DatabaseAgent class.
 * Wrapper of DatabaseHandler class.
 * @see DatabaseHandler
 */
class DatabaseAgent
{
private:
	DatabaseHandler _dbh;			/**< database connection handle.	*/

	int bufferQueryCount = 0;		/**< variable for counting number of queries in query buffer.	*/

	std::string queryBuffer = "";	/**< string variable keeping query buffer.	*/

public:

	/**
	* Class constructor.
	* @param[in] dbname the name of database file to be opened or created if not exist.
	*/
	DatabaseAgent(const char* dbname);

	/**
	* Class destructor.
	*/
	~DatabaseAgent();

	/**
	* Function used to execute SQL queries.
	* @param[in] query string with SQL query to be executed.
	* @param[in] callback callback function to process the result of executed SQL query.
	* @param[out] data pointer to variable that will be passed to callback function to hold SQL query result.
	*/
	void exec(std::string query, int(*callback)(void*, int, char**, char**), void* data);

	/**
	* Function adding SQL query to query buffer.
	* Use of this function highly increase speed of the program because SQLite3 database is optimized for transactions that include a number of SQL queries.
	* @param[in] query SQL query string.
	*/
	void execBuffered(std::string query);

	/**
	* Function to free the query buffer.
	* This function creates a SQLite3 transaction that include all SQL queries from query buffer.
	*/
	void freeBuffer(void);
};