/**
* @file FReader.h
*/

#pragma once
#include <vector>
#include <list>
#include "FileStructs.h"
#include "Database\DatabaseAgent.h"

/**
* CDB mesh files file extention.
*/
#define MESH_FILE_TYPE_CDB "cdb"

/**
* Default database file extention.
*/
#define DEFAULT_DATABASE_FILE_TYPE "pdb"

/**
* Default database file path.
*/
#define DEFAULT_DATABASE_FILE_PATH ""

/**
 * Maximum size of the list of faces kept in memory.
 */
#define FACE_LIST_MAX_SIZE 500

 /**
 * Maximum size of the list of named set faces kept in memory.
 */
#define NS_FACE_LIST_MAX_SIZE 100

 /**
  * FReader class. Main class of current solution. 
  * Handles .CDB mesh file format.
  */
class FReader
{
private:

	int modelType;									/**< variable holiding type of model. Values are: 0 - shell model, 1 - solid model. */
	
	int modelID = 1;								/**< variable holding model identificator.	*/

	FR_ERROR err;									/**< variable holiding error.	*/

	std::vector<_NodeS> _VSNode;					/**< vector of mesh nodes.	*/

	int ** nodeIDs;									/**< array of 2 elements for easy convertation from array indexes to node IDs.	*/

	std::vector<std::vector<short int>> _EdgesList;	/**< vector of node connections (edges).	*/

	std::vector<_FacesS> _FacesList;				/**< vector of faces nodes.	*/

	std::string filehash;							/**< variable holding md5 hash of mesh file.	*/

	DatabaseAgent * dba;							/**< pointer to database handle.	*/ 

	int lastFaceID = 0;								/**< variable of last given face ID. Used to give all faces unique IDs.	*/

	int lastNamedSetID = 0;							/**< variable of last given named set ID. Used to give all named sets unique IDs.	*/
	
	int lastNSFaceID = 0;							/**< variable of last given named set face ID. Used to give all named set faces unique IDs.	*/
	

	/**
	* function to check if string of block format is correct.
	* @param[in] str block format string.
	* @param[in] e_str block format template in regex format.
	* @return boolean result. true if string is coorect and false if not.
	*/
	bool checkBlockFormat(std::string str, const char * e_str);

	/**
	* function parsing format string of node block.
	* @param[in] str block format string.
	* @return _rowFormatS type result that include all gathered node block format information.
	* @see _rowFormatS
	*/
	_rowFormatS parseNodeBlockFormat(std::string str);

	/**
	* function parsing single row of node block.
	* @param[in] str node block row.
	* @param[in] rf node row format.
	* @return _NodeS type result that include all gathered node information.
	* @see _NodeS
	*/
	_NodeS parseNodeRow(std::string str, _rowFormatS rf);

	/**
	* function parsing format string of element block.
	* @param[in] str block format string.
	* @return _ElementsS type result that include all gathered element block format information.
	* @see _rowFormatS
	*/
	_rowFormatS parseElementBlockFormat(std::string str);

	/**
	* function parsing single row of element block.
	* @param[in] str node block row.
	* @param[in] rf node row format.
	* @return _ElementsS type result that include all gathered element information.
	* @see _ElementsS
	*/
	_ElementsS parseElementRow(std::string str, _rowFormatS rf, std::ifstream * f);
	
	/**
	* function counting amount of rows in whole block.
	* @param[in] fn file name.
	* @param[in] type block type.
	* @return _ElementsS type result that include all gathered element information.
	* @see FILE_BLOCK_TYPES
	*/
	int getBlockRowCount(const char* fn, FILE_BLOCK_TYPES type);

	/**
	* function checking if edge is already processed.
	* @param[in] v_from first node of the edge.
	* @param[in] v_to second node of the edge.
	* @return boolean result. true if edge is already processed, false if not.
	*/
	bool checkEdgeInList(unsigned int v_from, unsigned int v_to);

	/**
	* function adding single edge to _EdgesList and database.
	* @param[in] v_from first node of the edge.
	* @param[in] v_to second node of the edge.
	* @see _EdgesList
	*/
	void addToEdgeList(unsigned int v_from, unsigned int v_to);

	/**
	* function adding all edges of the element to _EdgesList and database.
	* @param[in] elem current mesh element.
	* @param[in] edges array of pairs of element node indexes which form an edge.
	* @param[in] n amount of edges to be added.
	* @see _ElementsS
	*/
	void addEdgesToEdgeList(_ElementsS elem, int edges[][2], int n);

	/**
	* function checking if face is already processed.
	* @param[in] nodeIDList first node of the edge.
	* @return _checkFaceResultS type result.
	* @see _checkFaceResultS
	*/
	_checkFaceResultS checkFaceInList(std::vector<int> nodeIDList);

	/**
	* function adding single face to _FacesList and database.
	* @param[in] nodeIDList vector of nodes which form face.
	* @param[in] elementID current mesh element ID.
	*/
	void addToFaceList(std::vector<int> nodeIDList, int elementID);

	/**
	* function adding multiple triangle faces to _FacesList and database.
	* @param[in] elem current mesh element.
	* @param[in] faces array of triplets of nodes which form a face.
	* @param[in] n amount of faces to be added.
	* @see _ElementsS
	*/
	void addTriFacesToFaceList(_ElementsS elem, int faces[][3], int n);

	/**
	* function adding multiple quad faces to _FacesList and database.
	* @param[in] elem current mesh element.
	* @param[in] faces array of quads of nodes which form a face.
	* @param[in] n amount of faces to be added.
	* @see _ElementsS
	*/
	void addQuadFacesToFaceList(_ElementsS elem, int faces[][4], int n);

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
	void parseElementToEdgesAndFaces(_ElementsS elem);
	
	/**
	* function computing md5 hash of file and passing it to filehash.
	* @param[in] fn file name.
	* @see filehash
	*/
	void getFileHash(const char* fn);

	/**
	* callback function holding result of getting md5 hash of previous mesh file with the same name from database.
	* Arguments are default for callback function of SQLite3 library.
	* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
	* @param[in] argc number of columns in the row.
	* @param[in] argv an array of strings represention fields in the row.
	* @param[in] azColName  an array of strings representing column names.
	*/
	static int modelSelectHashCallback(void *data, int argc, char **argv, char **azColName);

	/**
	* callback function holding result of getting element faces from database.
	* Arguments are default for callback function of SQLite3 library.
	* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
	* @param[in] argc number of columns in the row.
	* @param[in] argv an array of strings represention fields in the row.
	* @param[in] azColName  an array of strings representing column names.
	*/
	static int selectElementFacesCallback(void *data, int argc, char **argv, char **azColName);

	/**
	* callback function holding result of getting current model ID from database.
	* Arguments are default for callback function of SQLite3 library.
	* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
	* @param[in] argc number of columns in the row.
	* @param[in] argv an array of strings represention fields in the row.
	* @param[in] azColName  an array of strings representing column names.
	*/
	static int modelSelectIDCallback(void *data, int argc, char **argv, char **azColName);

	/**
	* callback function holding result of checking face of named set element.
	* Arguments are default for callback function of SQLite3 library.
	* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
	* @param[in] argc number of columns in the row.
	* @param[in] argv an array of strings represention fields in the row.
	* @param[in] azColName  an array of strings representing column names.
	*/
	static int checkFaceCallback(void *data, int argc, char **argv, char **azColName);

	/**
	* callback function holding result of getting face ID from database.
	* Arguments are default for callback function of SQLite3 library.
	* @param[out] data pointer to variable provided it the 4th argument of sqlite3_exec().
	* @param[in] argc number of columns in the row.
	* @param[in] argv an array of strings represention fields in the row.
	* @param[in] azColName  an array of strings representing column names.
	*/
	static int getFaceInDBCallback(void *data, int argc, char **argv, char **azColName);

	/**
	* function adding node to buffer for adding to database.
	* @param[in] v node to be added.
	* @see _NodeS
	*/
	void pushNodeToDB(_NodeS v);

	/**
	* function adding element to buffer for adding to database.
	* @param[in] e element to be added.
	* @see _ElementsS
	*/
	void pushElementToDB(_ElementsS e);

	/**
	* function adding element node to buffer for adding to database.
	* @param[in] nodeID ID of node to be added.
	* @param[in] elementID ID of element that node belongs to.
	*/
	void pushElementNodeToDB(int nodeID, int elementID);

	/**
	* function adding edge to buffer for adding to database.
	* @param[in] v1 first node of the edge.
	* @param[in] v2 second node of the edge.
	*/
	void pushEdgeToDB(int v1, int v2);

	/**
	* function checking existance of face in database.
	* @param[in] key the key of the face.	
	* @return integer result. If there is no face with such key in database function will return -1, else it will return face ID.
	* @see _faceKeyS
	*/
	int checkFaceInDB(_faceKeyS key);

	/**
	* function setting face status to internal in database.
	* @param[in] faceID ID internal face.
	*/
	void updateFaceInternal(int faceID, int elemID);

	/**
	* function adding face to buffer for adding to database.
	* @param[in] f face to be added.
	* @see _FacesS
	*/
	void pushFaceToDB(_FacesS fD);

	/**
	* function adding all faces from face buffer to database.
	*/
	void pushFaceListToDB();

	/**
	* function adding face node to buffer for adding to database.
	* @param[in] nodeID ID of node to be added.
	* @param[in] faceID ID of face that node belongs to.
	*/
	void pushFaceNodeToDB(int nodeID, int faceID);

	/**
	* function converting real node ID to internal index of this node in _VSNode.
	* @param[in] id ID of node.
	* @return integer result. Internal ID of node or -1 if it is not found.
	* @see _VSNode
	*/
	int getNodeIndex(int id);

	/**
	* function checking if list of faces in the face list.
	* It checks if list of faces of a single element on named set has already been proceeded
	* @param[in] _faces list of faces to be checked
	* @param[in] _NSFList pointer to the list of faces of named set to be checked in along with the database
	*/
	void checkFacesInList(std::vector<int> _faces, std::vector<_NamedSetFaceS>* _NSFList, DatabaseAgent * _dba);

public:

	/**
	* Class constructor.
	* @param[in] fn mesh file name.
	* @param[in] _dba database connection handle.
	* @see DatabaseAgent
	*/
	FReader(const char* fn, DatabaseAgent * _dba);

	/**
	* Class destructor.
	*/
	~FReader();	

	/**
	* Get error function.
	* @return error code.
	* @see FR_ERROR
	*/
	FR_ERROR getError(void);
};