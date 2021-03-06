#ifndef DRIVE_H
#define DRIVE_H

#include <pthread.h>
#include "robotapp.h"
#include "timestep.h"
#include "math.h"

#define TIMESLOT_LEN 10000  //Default timeslot: 10ms
#define ENCODER_MAX_POS_VAL 32768
#define ENCODER_MAX_NEG_VAL 32769

#ifdef __cplusplus	//Check if the compiler is C++
	extern "C"	//Code needs to be handled as C-style code
	{
#endif

/**
 * \brief Initialize drive system
 * \return
 * 0:OK \n
 * 1:Error \n
*/
int DriveInit(void);

pthread_t* _getDriveThread(void);

int AbortDriving(void);

int pauseDriving(void);

int continueDriving(void);

int WaitForDriving(void);

int getWheelPosition(int* posL, int* posR);

bool IsDriving(void);

/**
 * \brief Drive robot straight forward over specified distance with specified speed. Returns after drive is complete.
 * \param Distance :Distance to travel in mm
 * \param Speed :Speed in mm/s
*/
int DriveStraightDistance(float Distance,float Speed);
void* _DriveStraightDistance(void* args);

/**
 * \brief Rotate robot around right wheel over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateRWheel(float Angle,float Speed);
void* _DriveRotateRWheel(void* args);

/**
 * \brief Rotate robot around left wheel over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateLWheel(float Angle, float Speed);
void* _DriveRotateLWheel(void* args);

/**
 * \brief Rotate robot around center of wheelbase over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateCenter(float Angle, float Speed);
void* _DriveRotateCenter(void* args);

/**
 * \brief Follow line until specified distance has been traveled.
 * \param Distance :Distance to travel in mm
 * \param Speed :Speed in mm/s
*/
int DriveLineFollowDistance(int Distance, float Speed);
void* _DriveLineFollowDistance(void* args);

/**
 * \brief Follow line until end of line segment.
 * \param Speed :Speed in mm/s
*/
int DriveLineFollow(float Speed);
void* _DriveLineFollow(void* args);

/**
 * \brief Calibrate sensors for following a line.
*/
int Calibrate(void);
void* _Calibrate(void* args);

void _pauseDrivingLoop(sint16* lMotorAngle, sint16* rMotorAngle);

/**
 * \brief Set the left motor target (PID) with overflow protection of the encoders.
 * \param Angle :Angle in degrees
*/
void setLeftMotorEncoder(float Angle);

/**
 * \brief Set the right motor target (PID) with overflow protection of the encoders.
 * \param Angle :Angle in degrees
*/
void setRightMotorEncoder(float Angle);

#ifdef __cplusplus		//Check if the compiler is C++
	}		//End the extern "C" bracket
#endif

#endif
