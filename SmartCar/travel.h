#ifndef TRAVEL_H
#define TRAVEL_H

#include "robotapp.h"
#include "dijkstra.h"
#include "drive.h"
#include "msgqueue.h"

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

/**
 * \brief Travel through an array of nodes, precalculated by a path planning algorithm
 * \param Map :Pointer to node array
 * \param MapSize :Number of nodes in array
 * \param Start :Start node in path
 * \param Finish :End node in path
 * \param Speed :Travel speed
 * \return
 * 0:OK \n
 * 1:Error \n
*/
int Travel(NodeStruct *Map,int MapSize,int Start,int Finish,float Speed);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
