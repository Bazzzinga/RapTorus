/**
* @file DatabaseAgent.cpp
*/

#include "DatabaseAgent.h"

/**
* Class constructor.
* @param[in] dbname the name of database file to be opened or created if not exist.
*/
DatabaseAgent::DatabaseAgent(const char* dbname)
{
	queryBuffer.append(c_beginTransactionQuery); /**< Initializing query buffer. */
	_dbh.Init(dbname); /**< Initializing database handler. */
	
	this->err = _dbh.getError();
	if ((int)this->err.size() != 0)  /**< Handling  probable error. */
		return;

	/**
	 * Creating database tables if needed.
	 */	
	execBuffered(c_modelTableCreationQuery);
	execBuffered(c_nodesTableCreationQuery);	
	execBuffered(c_elementsTableCreationQuery);	
	execBuffered(c_elementNodesTableCreationQuery);	
	execBuffered(c_edgesTableCreationQuery);	
	execBuffered(c_facesTableCreationQuery);	
	execBuffered(c_faceNodesTableCreationQuery);
	execBuffered(c_NamedSetsTableCreationQuery);
	execBuffered(c_NamedSetItemsTableCreationQuery);
	execBuffered(c_NamedSetFacesTableCreationQuery);

	this->freeBuffer();
}

/**
* Class destructor.
*/
DatabaseAgent::~DatabaseAgent()
{

}

/**
* Function used to execute SQL queries.
* @param[in] query string with SQL query to be executed.
* @param[in] callback callback function to process the result of executed SQL query.
* @param[out] data pointer to variable that will be passed to callback function to hold SQL query result.
*/
void DatabaseAgent::exec(std::string query, int(*callback)(void*, int, char**, char**), void* data)
{
	_dbh.execQuery(query, callback, data);
}

/**
* Function adding SQL query to query buffer.
* Use of this function highly increase speed of the program because SQLite3 database is optimized for transactions that include a number of SQL queries.
* @param[in] query SQL query string.
*/
void DatabaseAgent::execBuffered(std::string query)
{
	++bufferQueryCount;
	
	queryBuffer.append(query).append(";");

	if (this->bufferQueryCount >= QUERY_BUFFER_MAX_SIZE)
		this->freeBuffer();

}

/**
* Function to free the query buffer.
* This function creates a SQLite3 transaction that include all SQL queries from query buffer.
*/
void DatabaseAgent::freeBuffer(void)
{
	queryBuffer.append(c_commitTransactionQuery);
	this->exec(queryBuffer, NULL, NULL);
	bufferQueryCount = 0;
	queryBuffer.clear();
	queryBuffer.append(c_beginTransactionQuery);
}

/**
* Get error function.
* @return error code.
* @see FR_ERROR
*/
std::string DatabaseAgent::getError(void)
{
	return this->err;
}