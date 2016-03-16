#ifndef DRIVE_H
#define DRIVE_H

#include "robotapp.h"
#include "timestep.h"
#include "math.h"
#include <pthread.h>

pthread_t driveThread;
pthread_mutex_t _driveThreadLock;

/**
 * \brief Initialize drive system
 * \return
 * 0:OK \n
 * 1:Error \n
*/
int DriveInit(void);

int AbortDriving(void);

int pauseDriving(void);

int continueDriving(void);

int WaitForDriving(void);

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
 * \brief Follow line until specified distance has been travelled
 * \param Distance :Distance to travel in mm
 * \param Speed :Speed in mm/s
*/
int DriveLineFollowDistance(int Distance, float Speed);
void* _DriveLineFollowDistance(void* args);

/**
 * \brief Follow line until end of line segment
 * \param Speed :Speed in mm/s
*/
int DriveLineFollow(float Speed);
void* _DriveLineFollow(void* args);

/**
 * \brief Calibrate sensors for following a line.
 * \return The correction factor to take into account.
*/
int calibrate(void);

void _pauseDrivingLoop(float* lMotorAngle, float* rMotorAngle);

#endif
