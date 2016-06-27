/**
* @file FileStructs.h
*/

#pragma once

#include <tuple>
#include <vector>

#define MAX_NUM_OF_NODE_COLS 8  /**< Maximum number of columns in elements' first row in element block. */

#define NAMED_SET_TYPE_NODE 0	/**< Nodal type of named sets identificator */   
#define NAMED_SET_TYPE_ELEM 1	/**< Elemental type of named sets identificator */ 

/**
 * An enum.
 * Contains identificators of different blocks of input file.
 */
enum FILE_BLOCK_TYPES
{
	CDB_NBLOCK, /**< Node information block in .CDB file. */
	CDB_EBLOCK  /**< Element information block in .CDB file. */
};

typedef std::vector<int> _faceKeyS;

/**
 * A structure.
 * Structure for mesh node.
 */
struct _NodeS
{
	unsigned int node_id;		/**< Node ID. */
	int solid_entity;			/**< Node solid entity ID. */
	int solid_line_location;	/**< Node solid line location. */

	double x[3] = { 0, 0, 0 };	/**< Node coordinates. */
	double a[3] = { 0, 0, 0 };	/**< Node rotation angels. */

	bool operator<(const _NodeS & a) const {
		return node_id < a.node_id;
	}
};

/**
* A structure.
* Structure for mesh element.
*/
struct _ElementsS
{
	unsigned short material;		/**< Element material ID. */
	unsigned short type;			/**< Element type ID. */
	unsigned short cnst;			/**< Element real constant ID. */
	unsigned short section;			/**< Element section ID. */
	unsigned short cs;				/**< Element coordinate system ID. */
	unsigned short life_flag;		/**< Element birth/death flag. */
	unsigned short reference;		/**< Element solid mode reference ID. */
	unsigned short shape;			/**< Element shape flag. */
	unsigned short nodes_num;		/**< Number of nodes defining this element. */
	unsigned int id;				/**< Element ID. */

	std::vector<int> nodeIDList;	/**< List of IDs of nodes defining this element. */
};

/**
* A structure.
* Structure for row format.
*/
struct _rowFormatS
{
	int row_num = 0;		/**< Number of columns containing integer numbers in a row. */
	int row_width = 0;		/**< Width of each integer column in the row. */
	int frow_num = 0;		/**< Number of floating numbers in a row. */
	int frow_width = 0;		/**< Width of each floating number column in the row. */
	int precision = 0;		/**< Precision of floating numbers. */
};

/**
* A structure.
* Structure for mesh face.
*/
struct _FacesS
{
	int id;					/**< Face Id. */

	_faceKeyS faceKey;		/**< Face key. */

	bool internal = false;	/**< Flag showing if this face is internal or not. */

	/**
	 * List of IDs of nodes defining this face.
	 * Node IDs are place according to right-hand rule.
	 */
	std::vector<_NodeS> nodeIDList;
};

/**
* A structure.
* Structure for result of checking face presence in buffer or database.
*/
struct _checkFaceResultS
{
	int i;			/**< Face index. */
	bool inDB;		/**< Flag of face presence in database or buffer. */
};

/**
 * An enum.
 * Contains identificators of probable module errors.
 */
enum FR_ERROR
{
	ERR_NO_ERR,				/**< No error. */
	ERR_OPEN,				/**< Error opening file. */
	ERR_WRONG_FILE_FORMAT,	/**< Wrong input file format. */
	ERR_OPENDB				/**< Error opening database. */
};

struct _EdgeS
{
	std::vector<double> x1;
	std::vector<double> x2;

	int id;
};

/**
* A structure.
* Structure for named set.
*/
struct _NamedSetS
{
	int SetID;				/**< Set ID. */
	std::string name;		/**< Set name. */
	int type;				/**< Set type. */
	int itemCount;			/**< Number of items in set. */

	std::vector<unsigned int> itemList;	/**< List of set items IDs. */
};

/**
* A structure.
* Structure for holding information of element type.
*/
struct _ElementTypeS
{
	int ID;					/**< Type ID. */
	int type;				/**< Type name. */
};

struct _PhysicalProperetiesS
{
	double E; //Elastic modulus
	double v; //Poisson koeff
};

struct _ConstraintsS
{
	enum Type
	{
		UX1 = 1 << 0,
		UX2 = 1 << 1,
		UX3 = 1 << 2,
		UX1X2 = UX1 | UX2,		
		UX1X3 = UX1 | UX3,
		UX2X3 = UX2 | UX3,
		UX1X2X3 = UX1 | UX2 | UX3
	};

	int nodeID;

	Type type;
};

struct _Load2DS
{
	int nodeID;

	double forceX1;
	double forceX2;
};

struct _Load3DS
{
	int nodeID;

	double forceX1;
	double forceX2;
	double forceX3;
};
