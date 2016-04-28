/**
* @file ElementTypes.h
*/

#pragma once

#define ELEM_TYPE_BRICK8  8  /**< Number of nodes in a linear brick. */
#define ELEM_SOLID185     8  /**< Number of nodes in a linear brick in ANSYS APDL notation. */
#define ELEM_TYPE_BRICK20 20 /**< Number of nodes in a quadratic brick. */
#define ELEM_SOLID186     20 /**< Number of nodes in a quadratic brick in ANSYS APDL notation. */

#define ELEM_TYPE_TETRA4  4  /**< Number of nodes in a linear tetrahedron. */
#define ELEM_SOLID285     4	 /**< Number of nodes in a linear tetrahedron in ANSYS APDL notation. */
#define ELEM_TYPE_TETRA10 10 /**< Number of nodes in a quadratic tetrahedron. */
#define ELEM_SOLID286     10 /**< Number of nodes in a quadratic tetrahedron in ANSYS APDL notation. */

#define ELEM_TYPE_185_BRICK8	185		/**< ID of type of a linear brick. */
#define ELEM_TYPE_186_BRICK20	186		/**< ID of type of a quadratic brick. */
#define ELEM_TYPE_285_TETRA4    285		/**< ID of type of a linear tetrahedron. */
#define ELEM_TYPE_187_TETRA10	187		/**< ID of type of a quadratic tetrahedron. */
#define ELEM_TYPE_182_PLANE4	182		/**< ID of type of a linear plane element. */
#define ELEM_TYPE_183_PLANE8_6  183		/**< ID of type of a quadratic plane element. */
#define ELEM_TYPE_188_BEAM2		188		/**< ID of type of a linear beam. */
#define ELEM_TYPE_189_BEAM3		189		/**< ID of type of a quadratic beam. */
#define ELEM_TYPE_181_SHELL4	181		/**< ID of type of a linear shell element. */
#define ELEM_TYPE_281_SHELL8	281		/**< ID of type of a quadratic shell element. */

#define ELEM_TYPE_NO_TYPE		0		/**< ID of empty type. */

/**
* Linear tetrahedron edges list
*/
//#define ELEM_TYPE_TETRA4_EDGES  { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 0 }, { 1, 2 }, { 1, 3 }, { 2, 1 }, { 2, 0 }, { 2, 3 }, { 3, 0 }, { 3, 2 }, { 3, 1 } } 
#define ELEM_TYPE_TETRA4_EDGES  { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } } 

/**
* Linear tetrahedron faces list
*/
#define ELEM_TYPE_TETRA4_FACES  { {0, 1, 3}, {1, 2, 3}, { 0, 2, 1 }, { 2, 0, 3 } }

/**
* Quadratic tetrahedron edges list
*/
//#define ELEM_TYPE_TETRA10_EDGES { { 0, 4 },{ 0, 6 },{ 0, 7 }, { 1, 4 }, { 1, 5 }, { 1, 8 }, { 2, 5 }, { 2, 6 }, { 2, 9 }, { 3, 7 }, { 3, 8 }, { 3, 9 }, { 4, 0 }, { 4, 1 }, { 5, 1 }, { 5, 2 }, { 6, 0 }, { 6, 2 }, { 7, 0 }, { 7, 3 }, { 8, 1 }, { 8, 3 }, { 9, 2 }, { 9, 3 } }
#define ELEM_TYPE_TETRA10_EDGES { { 0, 4 },{ 0, 6 },{ 0, 7 }, { 1, 4 }, { 1, 5 }, { 1, 8 }, { 2, 5 }, { 2, 6 }, { 2, 9 }, { 3, 7 }, { 3, 8 }, { 3, 9 } }

/**
* Quadratic tetrahedron faces list
*/
#define ELEM_TYPE_TETRA10_FACES { { 0, 1, 3 }, { 1, 2, 3 }, { 0, 2, 1 }, { 2, 0, 3 } }

/**
* Linear brick edges list
*/
//#define ELEM_TYPE_BRICK8_EDGES  { { 0, 1 },{ 0, 3 },{ 0, 4 }, { 1, 0 }, { 1, 2 }, { 1, 5 }, { 2, 1 }, { 2, 6 }, { 2, 3 }, { 3, 0 }, { 3, 2 }, { 3, 7 }, { 4, 0 }, { 4, 5 }, { 4, 7 }, { 5, 1 }, { 5, 4 }, { 5, 6 }, { 6, 2 }, { 6, 5 }, { 6, 7 }, { 7, 3 }, { 7, 4 }, { 7, 6 } }
#define ELEM_TYPE_BRICK8_EDGES  { { 0, 1 },{ 0, 3 },{ 0, 4 }, { 1, 2 }, { 1, 5 }, { 2, 6 }, { 2, 3 }, { 3, 7 }, { 4, 5 }, { 4, 7 }, { 5, 6 }, { 6, 7 } }

/**
* Linear brick faces list
*/
#define ELEM_TYPE_BRICK8_FACES  { { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 }, { 3, 0, 4, 7 }, { 4, 5, 6, 7 }, { 3, 2, 1, 0 } }

/**
* Quadratic brick edges list
*/
//#define ELEM_TYPE_BRICK20_EDGES { { 0,  8 },{ 0, 11 },{ 0, 16 }, { 1, 8 }, { 1,  9 }, { 1, 17 }, { 2,  9 }, { 2, 10 }, { 2, 18 }, { 3, 10 }, { 3, 11 }, { 3, 19 }, { 4, 12 }, { 4, 15 }, { 4, 16 }, { 5, 12 }, { 5, 13 }, { 5, 17 }, { 6, 13 }, { 6, 14 }, { 6, 18 }, { 7, 14 }, { 7, 15 }, { 7, 19 }, { 8,  0 }, { 8,  1 }, { 9,  1 }, { 9,  2 }, { 10, 2 }, { 10, 3 }, { 11, 0 }, { 11, 3 }, { 12, 4 }, { 12, 5 }, { 13, 5 }, { 13, 6 }, { 14, 6 }, { 14, 7 }, { 15, 7 }, { 15, 4 }, { 16, 0 }, { 16, 4 }, { 17, 1 }, { 17, 5 }, { 18, 2 }, { 18, 6 }, { 19, 3 }, { 19, 7 } }
#define ELEM_TYPE_BRICK20_EDGES { { 0,  8 },{ 0, 11 },{ 0, 16 }, { 1, 8 }, { 1,  9 }, { 1, 17 }, { 2,  9 }, { 2, 10 }, { 2, 18 }, { 3, 10 }, { 3, 11 }, { 3, 19 }, { 4, 12 }, { 4, 15 }, { 4, 16 }, { 5, 12 }, { 5, 13 }, { 5, 17 }, { 6, 13 }, { 6, 14 }, { 6, 18 }, { 7, 14 }, { 7, 15 }, { 7, 19 } }

/**
* Quadratic brick faces list
*/
#define ELEM_TYPE_BRICK20_FACES { { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 }, { 3, 0, 4, 7 }, { 4, 5, 6, 7 }, { 3, 2, 1, 0 } }

/**
* Linear plane element edges list
*/
//#define ELEM_TYPE_PLANE4_EDGES { {0, 1}, {0, 3}, {1, 2}, {2, 3}, {1, 0}, {3, 0}, {2, 1}, {3, 2} }
#define ELEM_TYPE_PLANE4_EDGES { {0, 1}, {0, 3}, {1, 2}, {2, 3} }

/**
* Linear plane element faces list
*/
#define ELEM_TYPE_PLANE4_FACES { {0, 1, 2, 3} }

/**
* Quadratic 8-node plane element edges list
*/
//#define ELEM_TYPE_PLANE8_EDGES { {0, 4}, {0, 7}, {1, 4}, {1, 5}, {2, 5}, {2, 6}, {3, 6}, {3, 7}, {4, 0}, {4, 1}, {5, 1}, {5, 2}, {6, 2}, {6, 3}, {7, 0}, {7, 3} }
#define ELEM_TYPE_PLANE8_EDGES { {0, 4}, {0, 7}, {1, 4}, {1, 5}, {2, 5}, {2, 6}, {3, 6}, {3, 7} }

/**
* Quadratic 8-node plane element faces list
*/
#define ELEM_TYPE_PLANE8_FACES { {0, 1, 2, 3} }

/**
* Quadratic 6-node plane element edges list
*/
//#define ELEM_TYPE_PLANE6_EDGES { {0, 3}, {0, 5}, {1, 3}, {1, 4}, {2, 4}, {2, 5}, {3, 0}, {3, 1}, {4, 1}, {4, 2}, {5, 0}, {5, 2} }
#define ELEM_TYPE_PLANE6_EDGES { {0, 3}, {0, 5}, {1, 3}, {1, 4}, {2, 4}, {2, 5} }

/**
* Quadratic 6-node plane element faces list
*/
#define ELEM_TYPE_PLANE6_FACES { {0, 1, 2} }

/**
* Linear beam element edges list
*/
//#define ELEM_TYPE_BEAM2_EDGES { {0, 1}, {1, 0} }
#define ELEM_TYPE_BEAM2_EDGES { {0, 1} }

/**
* Quadratic beam element edges list
*/
#define ELEM_TYPE_BEAM3_EDGES { {0, 2}, {1, 2}, {2, 0}, {2, 1} }
