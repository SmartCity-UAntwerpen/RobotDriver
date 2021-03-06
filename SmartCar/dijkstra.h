#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

#define NON -1
#define INF 10000
typedef struct NodeStruct
{
	int Neighbours[4];		//Index of neighbouring nodes, in order N,E,S,W   -1= no connection
	int Distance[4];		//Distance to neighbouring nodes, in order N,E,S,W
	char RFID[21];          //RFID-code of node

	//Solver variables
	char Visited;			//flag: visited or not
	int DV;                 //Distance value
	int Previous;           //Previous node
	int Next;               //Next node
	int NextAbsDir;         //Absolute direction to next node (0=N, 1=E, 2=S, 3=W)
	int NextRelDir;         //Relative direction to next node (-1:no direction, 1=turn left, 2=forward, 3=turn right)
} NodeStruct;

//Linked list for nodes
typedef struct Node
{
    int ID;
    NodeStruct* Node;
    struct Node* Next;
} Node;

/**
 * \brief Calculate path using dijkstra method
 * \param Map :Array of nodes describing the map
 * \param MapSize :Number of nodes in Map
 * \param Start :Start node index for the path to be calculated
 * \param Finish :Finish node index for the path to be calculated
 * \return
 * Path length expressed in number of nodes (not including start node)
*/
int Dijkstra(NodeStruct *Map,int MapSize,int Start,int Finish);

/**
 * \brief First step in the Dijkstra algorithm to calculate the shortest path from start to finish
 * \param Map :Array of nodes describing the map
 * \param MapSize :Number of nodes in Map
 * \param Start :Start node index for the path to be calculated
 * \param Finish :Finish node index for the path to be calculated
 */
void VisitNode(NodeStruct *Map, int MapSize, int Start, int Finish);

/**
 * \brief Second step in the Dijkstra algorithm to find the shortest path from finish to start
 * \param Map :Array of nodes describing the map
 * \param Finish :Finish node index for the path to be calculated
 * \return Path length expressed in number of nodes (not including start node)
 */
int TraceRoute(NodeStruct* Map, int Finish);

/**
 * \brief Get the direction to the next node relative to the previous node
 * \param PrevAbsDir :Absolute direction of the previous node entry point to the current node (0-3: N,E,S,W)
 * \param NextAbsDir :Absolute direction of the next node entry point to the current node (0-3: N,E,S,W)
 * \return Relative direction to the next node in respect to the previous node (0-3: N,E,S,W)
 */
int GetRelDirection(int PrevAbsDir, int NextAbsDir);

#ifdef __cplusplus	//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
