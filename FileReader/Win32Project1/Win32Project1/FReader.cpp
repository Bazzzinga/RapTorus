/**
* @file FReader.cpp
*/

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <iostream>
#include "FReader.h"
#include "FileStructs.h"
#include "ElementTypes.h"

#include "Database\md5\md5wrapper.h"

/**
* function computing md5 hash of file and passing it to filehash.
* @param[in] fn file name.
* @see filehash
*/
void FReader::getFileHash(const char* fn)
{
	md5wrapper md5;

	this->filehash = md5.getHashFromFile(std::string(fn));
}

/**
* callback function holding result of getting md5 hash of previous mesh file with the same name from database.
* Arguments are default for callback function of SQLite3 library.
* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
* @param[in] argc number of columns in the row.
* @param[in] argv an array of strings represention fields in the row.
* @param[in] azColName  an array of strings representing column names.
*/
int FReader::modelSelectHashCallback(void *data, int argc, char **argv, char **azColName)
{
	std::string* _hash_pointer = (std::string*)data;

	if (argc)
		*(_hash_pointer) = std::string(argv[0]);
	
	return 0;
}

/**
* callback function holding result of getting current model ID from database.
* Arguments are default for callback function of SQLite3 library.
* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
* @param[in] argc number of columns in the row.
* @param[in] argv an array of strings represention fields in the row.
* @param[in] azColName  an array of strings representing column names.
*/
int FReader::modelSelectIDCallback(void *data, int argc, char **argv, char **azColName)
{
	int* _modelID_pointer = (int*)data;

	if (argc)
		*(_modelID_pointer) = atoi(argv[0]);

	return 0;
}

/**
* function adding node to buffer for adding to database.
* @param[in] v node to be added.
* @see _NodeS
*/
void FReader::pushNodeToDB(_NodeS v)
{
	char * sqlQuery = sqlite3_mprintf(c_inputNode.c_str(), v.node_id, v.x[0], v.x[1], v.x[2], v.a[0], v.a[1], v.a[2], v.solid_entity, v.solid_line_location, this->modelID);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);
}

/**
* function adding element to buffer for adding to database.
* @param[in] e element to be added.
* @see _ElementsS
*/
void FReader::pushElementToDB(_ElementsS e)
{
	char * sqlQuery = sqlite3_mprintf(c_inputElement.c_str(), e.id, e.material, e.type, e.cnst, e.section, e.cs, e.life_flag, e.reference, e.shape, e.nodes_num, this->modelID);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);

	for (int i = 0; i < e.nodes_num; ++i)
		pushElementNodeToDB(e.nodeIDList[i], e.id);
}

/**
* function adding element node to buffer for adding to database.
* @param[in] nodeID ID of node to be added.
* @param[in] elementID ID of element that node belongs to.
*/
void FReader::pushElementNodeToDB(int nodeID, int elementID)
{
	char * sqlQuery = sqlite3_mprintf(c_inputElementNode.c_str(), nodeID, elementID);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);
}

/**
* function adding edge to buffer for adding to database.
* @param[in] v1 first node of the edge.
* @param[in] v2 second node of the edge.
*/
void FReader::pushEdgeToDB(int v1, int v2)
{
	char * sqlQuery = sqlite3_mprintf(c_inputEdge.c_str(), v1, v2);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);
}

/**
* Class constructor.
* @param[in] fn mesh file name.
* @param[in] _dba database connection handle.
* @see DatabaseAgent
*/
FReader::FReader(const char* fn, DatabaseAgent * _dba)
{
	/**
	 * Open input file.
	 */
	std::ifstream f(fn);	

	if (!f.good())
	{
		this->err = ERR_OPEN;
		return;
	}

	/**
	 * Variable to store read line.
	 */
	std::string line;

	/**
	 * Initializing class variables
	 */
	this->err = ERR_NO_ERR;
	this->dba = _dba;

	/**
	 * Evaluating md5 hash of input file.
	 */
	getFileHash(fn);

	/**
	 * Checking if the database is already filled and extract md5 hash of the file that was translated to database.
	 */
	std::string oldFileHash("");
	dba->exec(c_selectModelHash, modelSelectHashCallback, (void*)(&oldFileHash));

	if (oldFileHash.size() != 0)
	{
		if (oldFileHash.compare(this->filehash) == 0)
			return;
		
		/**
		 * If database is outdated it should be emptied.
		 */
		dba->exec(c_clearAllTables, NULL, NULL);
	}
	/**
	 *	Input file structure (CDB):
	 *	- Some unused information.
	 *	- Row starting with "NBLOCK" - begin of node information block.
	 *		Next row contains format of information table.
	 *		Format is (XiY,ZeU.W).
	 *		Where X - the number of first left columns. Y - width of these columns.
	 *		Z - the number of columns with floating-point numbers (first 3 - node coordinates, last 3 - rotating angles).
	 *		If row consist of less than Z columns the rest are taken as zeros.
	 *		U - width of these columns.
	 *		W - precision.
	 *		The block ends with "N,R5.3,LOC,  -1{,}" row.
	 *	- Row starting with "EBLOCK" - begin of element information block.
	 *		Next row contains format of information table.
	 *		Format is (XiY) - like NBLOCK fromat.
	 *		Every row has not more than X columns, the rest are on the next line.
	 *	    The block ends with "-1" row.
	 */

	/**
	 * Counting number of rows in node infortaion table.
	 */
	int nodeRowCount = getBlockRowCount(fn, CDB_NBLOCK);

	while (std::getline(f, line))
	{
		/**
		 * Node block information started.
		 */
		if (line.compare(0, 6, "NBLOCK") == 0)
		{
			bool end = false;
						
			std::string nb_format_line;

			/**
			 * Extracting model type.
			 */
			if (line.find("SOLID") != std::string::npos)
				this->modelType = 1;
			else
				this->modelType = 0;

			/**
			 * Writing model type into database.
			 */
			char * sqlQuery = sqlite3_mprintf(c_inputModel.c_str(), this->modelType, this->filehash.c_str());
			dba->exec(sqlQuery, NULL, NULL);
			sqlite3_free(sqlQuery);
			/**
			 * Getting model ID, witch was assigned to the model.
			 */
			int tempModelID;
			dba->exec(c_selectModelID, modelSelectIDCallback, (void*)(&tempModelID));
			this->modelID = tempModelID;

			/**
			 * Extracting next line of input file, that must contain information table format.
			 */
			if (!std::getline(f, nb_format_line))
				{ this->err = ERR_WRONG_FILE_FORMAT;return; }

			/**
			 * Checking if format line is correct.
			 */
			bool res = checkBlockFormat(nb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");
			if (!res) { this->err = ERR_WRONG_FILE_FORMAT; return; }

			/**
			 * Saving table format into variable.
			 * @see _rowFormatS
			 */
			_rowFormatS rowFormat = parseNodeBlockFormat(nb_format_line);

			/**
			 * Reading node information line by line from input file.
			 */
			do
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{	
					/**
					 * Checking if we came to the end of current block.
					 */
					if (nb_line.c_str()[0] == 'N')
						end = true;
					else
					{
						/**
						 * Parsing single row of information table and translating it into _NodeS variable.
						 * @see _NodeS
						 * @see parseNodeRow()
						 */
						_NodeS node = this->parseNodeRow(nb_line, rowFormat);

						/**
						 * Adding extracted node to node list.
						 * @see _VSNode
						 */
						_VSNode.push_back(node);

						/**
						 * Adding extracted node to database query buffer.
						 * @see pushNodeToDB()
						 */
						pushNodeToDB(node);
					}
				}
				else { this->err = ERR_WRONG_FILE_FORMAT; return; }
			} while (!end);

			/**
			 * Free database command buffer to execute all pending database queries.
			 */
			dba->freeBuffer();

			/**
			 * Creating support data structure for fast node access.
			 * @see nodeIDs
			 */
			nodeIDs = new int*[_VSNode.size()];
			for (int i = 0; i < (int)_VSNode.size(); ++i)
			{
				nodeIDs[i] = new int[2];
				nodeIDs[i][0] = i;
				nodeIDs[i][1] = _VSNode[i].node_id;
			}

			/**
			 * Creating support data structure to store edges, giving fast access to the edges without interaction with the database.
			 */
			_EdgesList = std::vector<std::vector<short int>>(_VSNode.size());
		}
		
		/**
		 * Mesh elements block information started.
		 */
		if (line.compare(0 ,6, "EBLOCK") == 0)
		{
			bool end = false;

			std::string eb_format_line;

			/**
			 * Extracting next line of input file, that must contain information table format.
			 */
			if (!std::getline(f, eb_format_line))
			{ this->err = ERR_WRONG_FILE_FORMAT; return; }

			/**
			 * Checking if format line is correct.
			 */
			bool res = checkBlockFormat(eb_format_line, "[(]([[:digit:]]+)i([[:digit:]]+)[)]");
			if (!res) { this->err = ERR_WRONG_FILE_FORMAT; return; }

			/**
			 * Saving table format into variable.
			 * @see _rowFormatS
			 */
			_rowFormatS rowFormat = parseElementBlockFormat(eb_format_line);

			/**
			 * Reading mesh element information line by line from input file.
			 */
			do
			{
				std::string nb_line;

				if (std::getline(f, nb_line))
				{
					/**
					 * Checking if we came to the end of current block.
					 */
					if ( (nb_line.size() == rowFormat.row_width) && (atoi(nb_line.c_str()) == -1 ))
						end = true;
					else
					{
						/**
						 * Parsing single row of information table and translating it into _NodeS variable.
						 * @see _ElementsS
						 * @see parseElementRow()
						 */
						_ElementsS elem = this->parseElementRow(nb_line, rowFormat, &f);

						/**
						 * Adding extracted element to database query buffer.
						 * @see pushElementToDB()
						 */
						pushElementToDB(elem);

						/**
						 * Parsing extracted element in order to get edges and faces it consist of.
						 * @see parseElementToEdgesAndFaces()
						 */
						parseElementToEdgesAndFaces(elem);
					}
				}
				else { this->err = ERR_WRONG_FILE_FORMAT; return; }
			} while (!end);
		}
						
	}

	/**
	 * Closing input file.
	 */
	f.close();

	return;
}

/**
* Class destructor.
*/
FReader::~FReader()
{
	/**
	 * Free all created with "new" command arrays:
	 * Two dimensional array nodeIDs and array _EdgesList.
	 */
	for (int i = 0; i < (int)_VSNode.size(); ++i)
		delete nodeIDs[i];
	
	delete nodeIDs;
}

/**
* function converting real node ID to internal index of this node in _VSNode.
* @param[in] id ID of node.
* @return integer result. Internal ID of node or -1 if it is not found.
* @see _VSNode
*/
int FReader::getNodeIndex(int id)
{
	/**
	 * Searching "id" in nodeIDs array using binary search algorithm.
	 */
	int l = 0;
	int r = _VSNode.size() - 1;
	
	while (l <= r)
	{
		int m = (l + r) / 2;
		if (nodeIDs[m][1] == id) return m;
		if (nodeIDs[m][1] < id) l = m + 1;
		else r = m - 1;
	}

	return -1;
}

/**
* function to check if string of block format is correct.
* @param[in] str block format string.
* @param[in] e_str block format template in regex format.
* @return boolean result. true if string is coorect and false if not.
*/
bool FReader::checkBlockFormat(std::string str, const char * e_str)
{
	std::regex exp(e_str);
	return std::regex_match(str, exp);
}

/**
* function parsing format string of node block.
* @param[in] str block format string.
* @return _rowFormatS type result that include all gathered node block format information.
* @see _rowFormatS
*/
_rowFormatS FReader::parseNodeBlockFormat(std::string str)
{
	/**
	 * Variable with string in regex format to get every item from input format string
	 * @see FReader()
	 */
	std::regex exp("[(]([[:digit:]]+)i([[:digit:]]+),([[:digit:]]+)e([[:digit:]]+).([[:digit:]]+)[)]");

	std::smatch match;
	std::string::const_iterator pos = str.cbegin();

	std::string temp;
	_rowFormatS res;

	/**
	 * Extracting every single item of format string.
	 */
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

/**
* function parsing single row of node block.
* @param[in] str node block row.
* @param[in] rf node row format.
* @return _NodeS type result that include all gathered node information.
* @see _NodeS
*/
_NodeS FReader::parseNodeRow(std::string str, _rowFormatS rf)
{
	/**
	 * Variable holding the future result.
	 */
	_NodeS res;

	/**
	 * Variable holding the current position in line.
	 */
	int pos = 0;

	/**
	 * Support string variable.
	 */
	std::string temp;

	/**
	 * Extracting first column of the row - node ID.
	 */
	temp = str.substr(pos, rf.row_width);
	res.node_id = atoi(temp.c_str());
	pos += rf.row_width;

	
	if (pos > (int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; } /**< Check if line is correct  */ // проверка правильности переданной строки

	if (this->modelType == 1) /**< SOLID model has two extra columns before coordinates */
	{
		temp = str.substr(pos, rf.row_width);
		res.solid_entity = atoi(temp.c_str());
		pos += rf.row_width;
		if (pos > (int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

		temp = str.substr(pos, rf.row_width);
		res.solid_line_location = atoi(temp.c_str());
		pos += rf.row_width;
		if (pos > (int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
	}

	for (int i = 0; i < 3; ++i) /**< Extracting coordinates */
		if (pos + rf.frow_width <= (int)str.size())
		{
			temp = str.substr(pos, rf.frow_width);
			res.x[i] = atof(temp.c_str());
			pos += rf.frow_width;
			if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
		}
	if (modelType == 1)
	{
		for (int j = 0; j < 3; ++j) /**< Extracting rotating angles */
			if (pos + rf.frow_width < (int)str.size())
			{
				temp = str.substr(pos, rf.frow_width);
				res.a[j] = atof(temp.c_str());
				pos += rf.frow_width;
				if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
			}
	}
	return res;
}

/**
 * function parsing format string of element block.
 * @param[in] str block format string.
 * @return _ElementsS type result that include all gathered element block format information.
 * @see _rowFormatS
 */
_rowFormatS FReader::parseElementBlockFormat(std::string str)
{
	/**
	 * Variable with string in regex format to get every item from input format string
	 * @see FReader()
	 */
	std::regex exp("[(]([[:digit:]]+)i([[:digit:]]+)[)]");

	std::smatch match;
	std::string::const_iterator pos = str.cbegin();

	std::string temp;
	_rowFormatS res;

	/**
	 * Extracting every single item of format string.
	 */
	std::regex_search(pos, str.cend(), match, exp);

	temp = match.str(1);
	res.row_num = atoi(temp.c_str());

	temp = match.str(2);
	res.row_width = atoi(temp.c_str());

	return res;
}

/**
 * function parsing single row of element block.
 * @param[in] str node block row.
 * @param[in] rf node row format.
 * @return _ElementsS type result that include all gathered element information.
 * @see _ElementsS
 */
_ElementsS FReader::parseElementRow(std::string str, _rowFormatS rf, std::ifstream * f)
{
	/**
	 * Variable holding the future result.
	 */
	_ElementsS res;

	/**
	 * Support string variable.
	 */
	std::string temp;

	/**
	 * Support string variable to hold the following line if it is needed.
	 */
	std::string second_line;
		
	/**
	 * Variable holding the current position in line.
	 */
	int pos = 0;

	/**
	 * Extracting element material ID.
	 */
	temp = str.substr(pos, rf.row_width);
	res.material = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element type.
	 */
	temp = str.substr(pos, rf.row_width);
	res.type = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element real constant number.
	 */
	temp = str.substr(pos, rf.row_width);
	res.cnst = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element section ID.
	 */
	temp = str.substr(pos, rf.row_width);
	res.section = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element coordinate system ID.
	 */
	temp = str.substr(pos, rf.row_width);
	res.cs = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element life/death flag.
	 */
	temp = str.substr(pos, rf.row_width);
	res.life_flag = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
	
	/**
	 * Extracting corresponding solid model number.
	 */
	temp = str.substr(pos, rf.row_width);
	res.reference = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	* Extracting element shape flag.
	*/
	temp = str.substr(pos, rf.row_width);
	res.shape = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element node number.
	 */
	temp = str.substr(pos, rf.row_width);
	res.nodes_num = atoi(temp.c_str());
	pos += rf.row_width;

	pos += rf.row_width;  /**< 10-th column is not used - skip it */
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Extracting element ID.
	 */
	temp = str.substr(pos, rf.row_width);
	res.id = atoi(temp.c_str());
	pos += rf.row_width;
	if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }

	/**
	 * Calculating amount of columns left in current line.
	 */
	int col_count = (res.nodes_num < MAX_NUM_OF_NODE_COLS)? res.nodes_num: MAX_NUM_OF_NODE_COLS;

	/**
	 * Extracting node IDs.
	 */
	for (int i = 0; i < col_count; ++i)
	{
		temp = str.substr(pos, rf.row_width);
		res.nodeIDList.push_back(atoi(temp.c_str()));
		pos += rf.row_width;
		if (pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
	}

	/**
	 * Checking if next line of input file belong to description of current element.
	 */
	if (res.nodes_num > MAX_NUM_OF_NODE_COLS)
	{
		std::getline(*f, second_line);

		int second_line_pos = 0;

		/**
		 * Calculating amount of columns in new line and extracting node IDs.
		 */
		for (int j = 0; j < res.nodes_num - MAX_NUM_OF_NODE_COLS; ++j)
		{
			temp = second_line.substr(second_line_pos, rf.row_width);
			res.nodeIDList.push_back(atoi(temp.c_str()));
			second_line_pos += rf.row_width;
			if (second_line_pos >(int)str.size()) { this->err = ERR_WRONG_FILE_FORMAT; return res; }
		}
	}

	return res;
}

/**
 * function counting amount of rows in whole block.
 * @param[in] fn file name.
 * @param[in] type block type.
 * @return _ElementsS type result that include all gathered element information.
 * @see FILE_BLOCK_TYPES
 */
int FReader::getBlockRowCount(const char* fn, FILE_BLOCK_TYPES type)
{
	std::ifstream f(fn); /**< variable holding input file stream. */
	std::string line; /**< string variable holding current file line. */
	std::string prev_line;  /**< string variable holding previous file line. */
	bool was_prev_line = false;  /**< boolean variable - flag of existance of previous read line. */
	int res = 0;  /**< variable counting amount of read block lines. */
	bool block_started = false;  /**<  boolean variable - flag of reading block of interest. */

	/**
	 * For CDB mesh file format node and element blocks have format line that we are not interested in when counting block items.
	 */
	if ((type == CDB_NBLOCK) || (type == CDB_EBLOCK))
		res = -1;

	/**
	 * Reading input file line by line.
	 */
	while (std::getline(f, line))
	{	
		if (block_started)
		{
			switch (type) /**< Checking for an end of block. */
			{
			case CDB_NBLOCK:
				if (line.c_str()[0] == 'N')
					return res;
				break;
			case CDB_EBLOCK:
				if (atoi(line.c_str()) == -1)
					return res;
				break;
			}
			
			/**
			 * Checking if current line is a continuation of previous one.
			 */
			if ((type == CDB_EBLOCK) && was_prev_line && (line.size() != prev_line.size())) 
			{
				prev_line = line;
				continue;
			}
			if (type == CDB_EBLOCK)
			{
				prev_line = line;
				was_prev_line = true;
			}
			res++;
		}
		else
			switch (type) /**< If its is not a target block yet checking if it is it's begining. */
			{
			case CDB_NBLOCK:
				if (line.compare(0, 6, "NBLOCK") == 0)
					block_started = true;
				break;
			case CDB_EBLOCK:
				if (line.compare(0, 6, "EBLOCK") == 0)
					block_started = true;
				break;
			}

	}

	f.close(); /**< Closing input file. */

	return res;
}
/**
* function parsing single mesh element.
* It parses it into all its edges and faces and then adding them to _EdgesList, _FacesList and database.
* @param[in] elem current mesh element.
* @see _EdgesList
* @see _FacesList
* @see addEdgesToEdgeList
* @see addTriFacesToFaceList
* @see addQuadFacesToFaceList
*/
void FReader::parseElementToEdgesAndFaces(_ElementsS elem)
{
	switch (elem.nodes_num)
	{
		case ELEM_TYPE_TETRA4:
		{
			/**
			 * Tetrahedra (with 4 nodes) nodes enumiration:
			 *
			 *       (3)------(2)
			 *      /   \  .  /
			 *     /    .\   /
			 *    /  .    \ /
			 *  (0)-------(1)
			 *
			 */

			 /**
			 * Pairs of node indexes that form an edge.
			 */
			int edgesToAdd[][2] = { { 0, 1 },{ 0, 2 },{ 0, 3 },
									{ 1, 0 },{ 1, 2 },{ 1, 3 },
									{ 2, 1 },{ 2, 0 },{ 2, 3 },
									{ 3, 0 },{ 3, 2 },{ 3, 1 } };
			

			addEdgesToEdgeList(elem, edgesToAdd, sizeof(edgesToAdd) / sizeof(int) / 2);

			/**
			* Triplets of node indexes that form a face.
			*/
			int facesToAdd[][3] = { {0, 1, 3},
									{1, 2, 3},
									{0, 2, 1},
									{2, 0, 3} };

			addTriFacesToFaceList(elem, facesToAdd, sizeof(facesToAdd) / sizeof(int) / 3);

			break;
		}
		case ELEM_TYPE_TETRA10:
		{
			/**
			* Tetrahedra (with 10 nodes) nodes enumiration:
			*
			*       (3)-(9)-(2)
			*      /   \  .  /
			*    (7)   (8) (5)          (0)---(6)---(2)
			*    /  .    \ /
			*  (0)--(4)--(1)
			*
			*/

			/**
			* Pairs of node indexes that form an edge.
			*/
			int edgesToAdd[][2] = { { 0, 4 },{ 0, 6 },{ 0, 7 },
									{ 1, 4 },{ 1, 5 },{ 1, 8 },
									{ 2, 5 },{ 2, 6 },{ 2, 9 },
									{ 3, 7 },{ 3, 8 },{ 3, 9 },
									{ 4, 0 },{ 4, 1 },
									{ 5, 1 },{ 5, 2 },
									{ 6, 0 },{ 6, 2 },
									{ 7, 0 },{ 7, 3 },
									{ 8, 1 },{ 8, 3 },
									{ 9, 2 },{ 9, 3 } };

			addEdgesToEdgeList(elem, edgesToAdd, sizeof(edgesToAdd) / sizeof(int) / 2);

			/**
			* Triplets of node indexes that form a face.
			*/
			int facesToAdd[][3] = { { 0, 1, 3 },
									{ 1, 2, 3 },
									{ 0, 2, 1 },
									{ 2, 0, 3 } };
			
			addTriFacesToFaceList(elem, facesToAdd, sizeof(facesToAdd) / sizeof(int) / 3);

			break;
		}
		case ELEM_TYPE_BRICK8:
		{
			/**
			* Brick (with 8 nodes) nodes enumiration:
			*
			*        (7)-------(6)
			*        /|        /|
			*       / |       / |
			*      /  |      /  |
			*    (4)-------(5)  |
			*     |   |     |   |
			*     |  (3)----|--(2)
			*     |  /      |  /
			*     | /       | /
			*     |/        |/
			*    (0)-------(1)
			*
			*/

			/**
			 * Pairs of node indexes that form an edge.
			 */
			int edgesToAdd[][2] = { { 0, 1 },{ 0, 3 },{ 0, 4 },
									{ 1, 0 },{ 1, 2 },{ 1, 5 },
									{ 2, 1 },{ 2, 6 },{ 2, 3 },
									{ 3, 0 },{ 3, 2 },{ 3, 7 },
									{ 4, 0 },{ 4, 5 },{ 4, 7 },
									{ 5, 1 },{ 5, 4 },{ 5, 6 },
									{ 6, 2 },{ 6, 5 },{ 6, 7 },
									{ 7, 3 },{ 7, 4 },{ 7, 6 } };

			addEdgesToEdgeList(elem, edgesToAdd, sizeof(edgesToAdd) / sizeof(int) / 2);

			/**
			* Qauds of node indexes that form a face.
			*/
			int facesToAdd[][4] = { { 0, 1, 5, 4 },
									{ 1, 2, 6, 5 },
									{ 2, 3, 7, 6 },
									{ 3, 0, 4, 7 },
									{ 4, 5, 6, 7 },
									{ 3, 2, 1, 0 } };

			addQuadFacesToFaceList(elem, facesToAdd, sizeof(facesToAdd) / sizeof(int) / 4);

			break;
		}
		case ELEM_TYPE_BRICK20:
		{
			/**
			* Brick (with 8 nodes) nodes enumiration:
			*
			*        (7)--(14)-(6)
			*        /|        /|
			*     (15)|     (13)|
			*      / (19)    / (18)
			*    (4)--(12)-(5)  |
			*     |   |     |   |
			*     |  (3)-(10|--(2)
			*   (16) /    (17) /
			*     |(11)     |(9)
			*     |/        |/
			*    (0)--(8)--(1)
			*
			*/

			/**
			* Pairs of node indexes that form an edge.
			*/
			int edgesToAdd[][2] = { { 0,  8 },{ 0, 11 },{ 0, 16 },
									{ 1,  8 },{ 1,  9 },{ 1, 17 },
									{ 2,  9 },{ 2, 10 },{ 2, 18 },
									{ 3, 10 },{ 3, 11 },{ 3, 19 },
									{ 4, 12 },{ 4, 15 },{ 4, 16 },
									{ 5, 12 },{ 5, 13 },{ 5, 17 },
									{ 6, 13 },{ 6, 14 },{ 6, 18 },
									{ 7, 14 },{ 7, 15 },{ 7, 19 },
									{ 8,  0 },{ 8,  1 },
									{ 9,  1 },{ 9,  2 },
									{ 10, 2 },{ 10, 3 },
									{ 11, 0 },{ 11, 3 },
									{ 12, 4 },{ 12, 5 },
									{ 13, 5 },{ 13, 6 },
									{ 14, 6 },{ 14, 7 },
									{ 15, 7 },{ 15, 4 },
									{ 16, 0 },{ 16, 4 },
									{ 17, 1 },{ 17, 5 },
									{ 18, 2 },{ 18, 6 },
									{ 19, 3 },{ 19, 7 } };

			addEdgesToEdgeList(elem, edgesToAdd, sizeof(edgesToAdd) / sizeof(int) / 2);

			/**
			* Qauds of node indexes that form a face.
			*/
			int facesToAdd[][4] = { { 0, 1, 5, 4 },
									{ 1, 2, 6, 5 },
									{ 2, 3, 7, 6 },
									{ 3, 0, 4, 7 },
									{ 4, 5, 6, 7 },
									{ 3, 2, 1, 0 } };

			addQuadFacesToFaceList(elem, facesToAdd, sizeof(facesToAdd) / sizeof(int) / 4);

			break;
		}
	}
}

/**
* function checking if edge is already processed.
* @param[in] v_from first node of the edge.
* @param[in] v_to second node of the edge.
* @return boolean result. true if edge is already processed, false if not.
*/
bool FReader::checkEdgeInList(unsigned int v_from, unsigned int v_to)
{
	/**
	 * Singularity check.
	 */
	if (v_from == v_to)
		return true;

	/**
	* Get internal index of first node.
	* @see getNodeIndex()
	*/
	int index = getNodeIndex(v_from);

	/**
	* If there is no edges from this node, then there is definitely no edge from v_from to v_to
	*/
	if (_EdgesList[index].size() == 0)
		return false;

	/**
	* Looking for v_to node in v_from neighbours
	*/
	for (int i = 0; i < (int)_EdgesList[index].size(); ++i)
		if (_EdgesList[index][i] == v_to)
			return true;
	
	return false;
}

/**
* function adding single edge to _EdgesList and database.
* @param[in] v_from first node of the edge.
* @param[in] v_to second node of the edge.
* @see _EdgesList
* @see checkEdgeInList()
* @see pushEdgeToDB()
*/
void FReader::addToEdgeList(unsigned int v_from, unsigned int v_to)
{
	if (!checkEdgeInList(v_from, v_to))
	{
		_EdgesList[getNodeIndex(v_from)].push_back(v_to);
		if(!checkEdgeInList(v_to, v_from))
			pushEdgeToDB(v_from, v_to);
	}
}

/**
* function adding all edges of the element to _EdgesList and database.
* @param[in] elem current mesh element.
* @param[in] edges array of pairs of element node indexes which form an edge.
* @param[in] n amount of edges to be added.
* @see _ElementsS
* @see addToEdgeList()
*/
void FReader::addEdgesToEdgeList(_ElementsS elem, int edges[][2], int n)
{
	for (int i = 0; i < n; ++i)
		addToEdgeList(elem.nodeIDList[edges[i][0]], elem.nodeIDList[edges[i][1]]);
}

/**
 * Support function.
 * This function is not part of FReader class. It is used to convert vector of node IDs that form face into a _faceKeyS.
 * @param[in] nodeIDList vector of node IDs that form a face.
 * @return _faceKeyS structure.
 * @see _faceKeyS
 */
_faceKeyS convertVectorToFaceKeyS(std::vector<int> nodeIDList)
{
	_faceKeyS res;

	res.v1 = nodeIDList[0];
	res.v2 = nodeIDList[1];
	res.v3 = nodeIDList[2];

	if (nodeIDList.size() > 3)
		res.v4 = nodeIDList[3];

	return res;
}

/**
* callback function holding result of getting face ID from database.
* Arguments are default for callback function of SQLite3 library.
* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
* @param[in] argc number of columns in the row.
* @param[in] argv an array of strings represention fields in the row.
* @param[in] azColName  an array of strings representing column names.
*/
int FReader::getFaceInDBCallback(void *data, int argc, char **argv, char **azColName)
{
	int* _FaceID_pointer = (int*)data;

	if (argc)
		*(_FaceID_pointer) = atoi(argv[0]);

	return 0;
}

/**
* function checking existance of face in database.
* @param[in] key the key of the face.
* @return integer result. If there is no face with such key in database function will return -1, else it will return face ID.
* @see _faceKeyS
*/
int FReader::checkFaceInDB(_faceKeyS key)
{
	/**
	 * Calculating a face key for database query.
	 */
	std::string key_s = "";
	key_s.append(std::to_string(key.v1)).append(".").append(std::to_string(key.v2)).append(".").append(std::to_string(key.v3)).append(".").append(std::to_string(key.v4));

	char * sqlQuery = sqlite3_mprintf(c_selectFaceByKey.c_str(), key_s.c_str());
	
	int faceID = -1;

	dba->exec(std::string(sqlQuery), getFaceInDBCallback, (void*)(&faceID));
		
	sqlite3_free(sqlQuery);

	return faceID;
}

/**
* function checking if face is already processed.
* @param[in] nodeIDList first node of the edge.
* @return _checkFaceResultS type result.
* @see _checkFaceResultS
*/
_checkFaceResultS FReader::checkFaceInList(std::vector<int> nodeIDList)
{	
	_checkFaceResultS res;

	res.inDB = false;
	res.i = -1;

	std::sort(nodeIDList.begin(), nodeIDList.end()); /**< Sorting node ID list to convert it into a face key. */

	_faceKeyS key = convertVectorToFaceKeyS(nodeIDList);

	int checkInDB = checkFaceInDB(key); /**< Checking if face is presented in database. */

	if (checkInDB != -1)
	{
		res.i = checkInDB;
		res.inDB = true;
		return res;
	}
	
	/**
	 * Checking if face is presented in face buffer.
	 */
	for (int i = 0; i < (int)_FacesList.size(); ++i)
		if ((!_FacesList[i].internal) && (key == _FacesList[i].faceKey))
		{
			res.i = i;
			return res;
		}

	return res;
}

/**
* function setting face status to internal in database.
* @param[in] faceID ID internal face.
*/
void FReader::updateFaceInternal(int faceID)
{
	char * sqlQuery = sqlite3_mprintf(c_updateFaceInternal.c_str(), faceID);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);
}

/**
* function adding face to buffer for adding to database.
* @param[in] f face to be added.
* @param[in] elementID ID of the element that face belongs to.
* @see _FacesS
*/
void FReader::pushFaceToDB(_FacesS f, int elementID)
{
	/**
	* Calculating a face key for database query.
	*/
	std::string key_s = "";
	key_s.append(std::to_string(f.faceKey.v1)).append(".").append(std::to_string(f.faceKey.v2)).append(".").append(std::to_string(f.faceKey.v3)).append(".").append(std::to_string(f.faceKey.v4));


	char * sqlQuery = sqlite3_mprintf(c_insertFace.c_str(), f.id, elementID, key_s.c_str());

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);

	/**
	 * Adding relations between face and its nodes to database.
	 * @see pushFaceNodeToDB()
	 */
	for (int i = 0; i < (int)f.nodeIDList.size(); ++i)
		pushFaceNodeToDB(f.nodeIDList[i], f.id);
}

/**
* function adding face node to buffer for adding to database.
* @param[in] nodeID ID of node to be added.
* @param[in] faceID ID of face that node belongs to.
*/
void FReader::pushFaceNodeToDB(int nodeID, int faceID)
{
	char * sqlQuery = sqlite3_mprintf(c_insertFaceNode.c_str(), faceID, nodeID);

	dba->execBuffered(std::string(sqlQuery));

	sqlite3_free(sqlQuery);
}

/**
* function adding single face to _FacesList and database.
* @param[in] nodeIDList vector of nodes which form face.
* @param[in] elementID current mesh element ID.
*/
void FReader::addToFaceList(std::vector<int> nodeIDList, int elementID)
{	
	_checkFaceResultS check_res = checkFaceInList(nodeIDList);
	
	int index = check_res.i;
	/**
	 * If face was found then update it's state as internal face.
	 */
	if (index != -1)
	{
		if (check_res.inDB)
			updateFaceInternal(index);
		else
		{
			_FacesList[index].internal = true;
			updateFaceInternal(_FacesList[index].id);
		}
	}
	else
	{
		_FacesS temp;

		++lastFaceID;

		temp.id = lastFaceID;

		temp.nodeIDList = nodeIDList;

		std::sort(nodeIDList.begin(), nodeIDList.end());

		temp.faceKey = convertVectorToFaceKeyS(nodeIDList);

		_FacesList.push_back(temp);

		/**
		 * If face buffers list has reached its maximum size than empty it, writing all its items into database.
		 */
		if ((int)_FacesList.size() >= FACE_LIST_MAX_SIZE)
		{
			for (int i = 0; i < (int)_FacesList.size(); ++i)
				pushFaceToDB(_FacesList[i], elementID);

			dba->freeBuffer();
			_FacesList.clear();
		}
	}
}

/**
* function adding multiple triangle faces to _FacesList and database.
* @param[in] elem current mesh element.
* @param[in] faces array of triplets of nodes which form a face.
* @param[in] n amount of faces to be added.
* @see _ElementsS
*/
void FReader::addTriFacesToFaceList(_ElementsS elem, int faces[][3], int n)
{
	for (int i = 0; i < n; ++i)
	{
		std::vector<int> temp;

		for (int j = 0; j < 3; ++j)
			temp.push_back(elem.nodeIDList[faces[i][j]]);

		/**
		 * Singularity check.
		 */
		if( !( (temp[0] == temp[1]) || (temp[2] == temp[1]) || (temp[0] == temp[2])))
			addToFaceList(temp, elem.id);
	}
}

/**
* function adding multiple quad faces to _FacesList and database.
* @param[in] elem current mesh element.
* @param[in] faces array of quads of nodes which form a face.
* @param[in] n amount of faces to be added.
* @see _ElementsS
*/
void FReader::addQuadFacesToFaceList(_ElementsS elem, int faces[][4], int n)
{
	for (int i = 0; i < n; ++i)
	{
		std::vector<int> temp;

		for (int j = 0; j < 4; ++j)
			temp.push_back(elem.nodeIDList[faces[i][j]]);

		/**
		 * Singularity check.
		 */		
		if( !( (temp[0] == temp[2]) || (temp[1] == temp[3]) ) )
			addToFaceList(temp, elem.id);
	}
}

/**
* Get error function.
* @return error code.
* @see FR_ERROR
*/
FR_ERROR FReader::getError(void)
{
	return this->err;
}