#ifndef TAGREADER_H
#define TAGREADER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

/**
 * \brief Scan for passive tag in range, return 7 byte UID
 * \param data :Location to store tag UID. Buffer must be 21 bytes long at least. UID is stored as a NULL terminated string
 * \return
 * 0:OK \n
 * 1:Tag reader error \n
 * 2:No tag detected \n
 * 3:Other error \n
*/
int TagReaderGetUID(char *data);

/**
 * \brief Write byte data to a passive tag in range
 * \param blockNumber :The starting block to be updated
 * \param dataLength :The number of bytes to be updated
 * \param data :New data to store on the tag.
 * \return
 * 0:OK \n
 * 1:Tag reader error \n
 * 2:No tag detected \n
 * 3:Other error \n
*/
int TagReaderWriteData(int blockNumber, int dataLength, char* data);

/**
 * \brief Read byte data from a passive tag in range
 * \param blockNumber :The starting block to be readed from
 * \param data :Location to store the data
 * \param dataLength :The number of bytes to read
 * 0:OK \n
 * 1:Tag reader error \n
 * 2:No tag detected \n
 * 3:Other error \n
*/
int TagReaderReadData(int blockNumber, int dataLength, char* data);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
