#pragma once

#include <string>

const std::string c_beginTransactionQuery = "BEGIN TRANSACTION;";

const std::string c_commitTransactionQuery = "COMMIT TRANSACTION;";

const std::string c_modelTableCreationQuery = "CREATE TABLE IF NOT EXISTS Model (ModelType INTEGER, ModelID INTEGER PRIMARY KEY AUTOINCREMENT, ModelHash VARCHAR (33));";

const std::string c_nodesTableCreationQuery = "CREATE TABLE IF NOT EXISTS  Nodes (NodeID INTEGER PRIMARY KEY, x DOUBLE, y DOUBLE, z DOUBLE, a1 DOUBLE, a2 DOUBLE, a3 DOUBLE, solidEntity INTEGER, solidLineLocation INTEGER, ModelID REFERENCES Model (ModelID) ON DELETE CASCADE);";

const std::string c_elementsTableCreationQuery = "CREATE TABLE IF NOT EXISTS Elements (ElementID INTEGER PRIMARY KEY, material INTEGER, type INTEGER, cnst INTEGER, section INTEGER, cs INTEGER, life_flag INTEGER, reference INTEGER, shape INTEGER, nodes_sum INTEGER, ModelID INTEGER REFERENCES Model (ModelID)  ON DELETE CASCADE);";

const std::string c_elementNodesTableCreationQuery = "CREATE TABLE IF NOT EXISTS ElementNodes (NodeID INTEGER REFERENCES Nodes (NodeID) ON DELETE CASCADE, ElementID INTEGER REFERENCES Elements (ElementID) ON DELETE CASCADE);";

const std::string c_edgesTableCreationQuery = "CREATE TABLE IF NOT EXISTS Edges (EdgeID INTEGER PRIMARY KEY AUTOINCREMENT, Node1ID INTEGER REFERENCES Nodes (NodeID) ON DELETE CASCADE, Node2ID INTEGER REFERENCES Nodes (NodeID)  ON DELETE CASCADE);";

const std::string c_facesTableCreationQuery = "CREATE TABLE IF NOT EXISTS Faces (FaceID INTEGER PRIMARY KEY, ElementID INTEGER REFERENCES Elements (ElementID) ON DELETE CASCADE, internal BOOLEAN DEFAULT FALSE, FaceKey VARCHAR (50));";

const std::string c_faceNodesTableCreationQuery = "CREATE TABLE IF NOT EXISTS FaceNodes (FaceID INTEGER REFERENCES Faces (FaceID) ON DELETE CASCADE, NodeID INTEGER REFERENCES Nodes (NodeID)  ON DELETE CASCADE);";

const std::string c_NamedSetsTableCreationQuery = "CREATE TABLE IF NOT EXISTS NamedSets (SetID INTEGER PRIMARY KEY, Name VARCHAR (50), Type INTEGER, ItemCount INTEGER, ModelID REFERENCES Model (ModelID) ON DELETE CASCADE);";

const std::string c_NamedSetItemsTableCreationQuery = "CREATE TABLE NamedSetItems (SetID INTEGER REFERENCES NamedSets (SetID), ItemID INTEGER);";

const std::string c_selectModelHash = "SELECT ModelHash FROM Model";

const std::string c_selectModelID = "SELECT ModelID FROM Model";

const std::string c_clearAllTables = "DELETE FROM Model; DELETE FROM Nodes; DELETE FROM Elements; DELETE FROM ElementNodes; DELETE FROM Edges; DELETE FROM Faces; DELETE FROM FaceNodes; VACUUM;";

const std::string c_inputModel = "INSERT INTO Model (ModelType, ModelHash) VALUES (%d, '--')";

const std::string c_updateModelHash = "UPDATE Model SET ModelHash=%Q WHERE ModelID=%d";

const std::string c_inputNode = "INSERT INTO Nodes (NodeID, x, y, z, a1, a2, a3, solidEntity, solidLineLocation, ModelID) VALUES (%d, %E, %E, %E, %E, %E, %E, %d, %d, %d)";

const std::string c_inputElement = "INSERT INTO Elements (ElementID, material, type, cnst, section, cs, life_flag, reference, shape, nodes_sum, ModelID) VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)";

const std::string c_inputElementNode = "INSERT INTO ElementNodes (NodeID, ElementID) VALUES (%d, %d)";

const std::string c_selectEdgeID = "SELECT * FROM Edges WHERE (Node1ID = %d AND Node2ID = %d) OR (Node1ID = %d AND Node2ID = %d)";

const std::string c_inputEdge = "INSERT INTO Edges (Node1ID, Node2ID) VALUES (%d, %d)";

const std::string c_selectFaceByKey = "SELECT FaceID FROM Faces WHERE (internal = 0) AND (FaceKey = %Q)";

const std::string c_updateFaceInternal = "UPDATE Faces SET internal = 1 WHERE FaceID = %d";

const std::string c_insertFace = "INSERT INTO Faces (FaceID, ElementID, internal, FaceKey) VALUES (%d, %d, %d, %Q)";

const std::string c_insertFaceNode = "INSERT INTO FaceNodes (FaceID, NodeID) VALUES (%d, %d)";

const std::string c_insertNamedSet = "INSERT INTO NamedSets (SetID, Name, Type, ItemCount, ModelID) VALUES (%d, %Q, %d, %d, %d)";

const std::string c_insertNamedSetItem = "INSERT INTO NamedSetItems (SetID, ItemID) VALUES (%d, %d)";