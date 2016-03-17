#ifndef MAP_H
#define MAP_H

#include <string.h>
#include "dijkstra.h"

int parseMapFile(char* fileName);

int clearRouteMap(NodeStruct* roadMap);

int processNodes(FILE* mapFile, NodeStruct* roadMap);

int processDubiousLine(FILE* mapFile);

/**
 * \brief Add a new node to the NodeStruct for initialisation of the map for the Dijkstra algorithm
 * \param Map :Array of nodes describing the map
 * \param map :Index for the new node to be initialized
 * \param n :Index of the neighbour in the north (-1: no neighbour)
 * \param o :Index of the neighbour in the east (-1: no neighbour)
 * \param z :Index of the neighbour in the south (-1: no neighbour)
 * \param w :Index of the neighbour in the west (-1: no neighbour)
 * \param nn :Cost of the route to the north
 * \param oo :Cost of the route to the east
 * \param zz :Cost of the route to the south
 * \param ww :Cost of the route to the west
 * \param RFID :RFID code of the node
 */
void InitMap(NodeStruct* Map, int map, int n, int o, int z, int w, int nn, int oo, int zz, int ww, char* RFID);

NodeStruct* getRoadMap(void);

int getRoadMapSize(void);

#endif
