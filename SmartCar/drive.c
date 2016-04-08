/**
    Code developed by Quinten Van Hasselt & Timothy Verstraete
    Modified by Thomas Huybrechts & Arthur Janssens
**/

#include "drive.h"
#include "math.h"
#include "timestep.h"

static pthread_t driveThread;
static pthread_mutex_t _driveThreadLock;

static float const MMPD = 0.46;     // mm per degree
static float const MMRD = 2;        // mm per degree to rotate
static uint16 const KP = 1800;      // first value was 2100, this one seems more stable
static uint16 const KD = 4200;      // first value was 5000, this one seems more stable
static uint16 const KI = 0;
static uint16 const IMAX = 0;
static int const MOTOR_R = 1;       // motor right
static int const MOTOR_L = 2;       // motor left
static int const SENSOR_R = 1;      // sensor right
static int const SENSOR_L = 2;      // sensor left
int cal;                            // calibration difference value
uint16 iLcal;                       // left sensor calibrated value on drive stroke
uint16 iRcal;                       // right sensor calibrated value on drive stroke

static float _Distance;
static float _Angle;
static float _Speed;
static bool _driveThreadRunning = false;
static bool _cancelDriving = false;
static bool _pauseDriving = false;

/**
 * \brief Initialize drive system
 * \return
 * 0:OK \n
 * 1:Error \n
*/
int DriveInit(void)
{
    int error = 0;

    //Setup drive motors
    error += LegoMotorSetup(&LegoMotor,MOTOR_R,0,0);
    error += LegoMotorSetup(&LegoMotor,MOTOR_L,0,0);
    error += LegoMotorDirectControl(&LegoMotor, MOTOR_R, 0);
    error += LegoMotorDirectControl(&LegoMotor, MOTOR_L, 0);
    error += LegoMotorPosPIDControl(&LegoMotor, MOTOR_R, 0);
    error += LegoMotorPosPIDControl(&LegoMotor, MOTOR_L, 0);
    error += LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    error += LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    //Setup reflection sensors
    error += LegoSensorSetup(&LegoSensor, SENSOR_R, CFG_LSENSOR);
    error += LegoSensorSetup(&LegoSensor, SENSOR_L, CFG_LSENSOR);
    error += LegoSensorSetupLSensor(&LegoSensor, SENSOR_R, 1);
    error += LegoSensorSetupLSensor(&LegoSensor, SENSOR_L, 1);

    //Calibrate
    cal = calibrate();
    if(error == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

pthread_t* _getDriveThread(void)
{
    return &driveThread;
}

int AbortDriving(void)
{
    int res;

    if(_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        printf("ABORT driving!\n");

        //Set the cancel driving flag high
        _cancelDriving = true;

        pthread_join(driveThread, NULL);

        //Shutdown motors
        res=LegoMotorSetup(&LegoMotor,1,0,0);
        if (res>0) printf("Abort handler: LegoMotorSetup() CH1 fail.\n");
        res=LegoMotorSetup(&LegoMotor,2,0,0);
        if (res>0) printf("Abort handler: LegoMotorSetup() CH2 fail.\n");
        res=LegoMotorSetup(&LegoMotor,3,0,0);
        if (res>0) printf("Abort handler: LegoMotorSetup() CH3 fail.\n");
        res=LegoMotorDirectControl(&LegoMotor,1,0);
        if (res>0) printf("Abort handler: LegoMotorDirectControl() CH1 fail.\n");
        res=LegoMotorDirectControl(&LegoMotor,2,0);
        if (res>0) printf("Abort handler: LegoMotorDirectControl() CH2 fail.\n");
        res=LegoMotorDirectControl(&LegoMotor,3,0);
        if (res>0) printf("Abort handler: LegoMotorDirectControl() CH3 fail.\n");

        _driveThreadRunning = false;
        _cancelDriving = false;
        _pauseDriving = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

int pauseDriving(void)
{
    if(_driveThreadRunning && !_pauseDriving)
    {
        _pauseDriving = true;

        return 0;
    }

    return 1;
}

int continueDriving(void)
{
    if(_driveThreadRunning && _pauseDriving)
    {
        _pauseDriving = false;

        return 0;
    }

    return 1;
}

int WaitForDriving(void)
{
    if(_driveThreadRunning)
    {
        if(pthread_join(driveThread, NULL) != 0)
        {
            return 2;
        }

    }
    else
    {
        return 1;
    }

    return 0;
}

int getWheelPosition(int* posL, int* posR)
{
    sint16 encodeL, encodeR;

    //Get encoder position of motors;
    LegoMotorGetPos(&LegoMotor, MOTOR_L, &encodeL);
    LegoMotorGetPos(&LegoMotor, MOTOR_R, &encodeR);

    //Calculate position in mm
    *posL = encodeL * MMPD/2;
    *posR = encodeR * MMPD/2;

    return (*posL + *posR)/2;
}

bool IsDriving(void)
{
    return _driveThreadRunning;
}

/**
 * \brief Drive robot straight forward over specified distance with specified speed. Returns after drive is complete.
 * \param Distance :Distance to travel in mm
 * \param Speed :Speed in mm/s
*/
int DriveStraightDistance(float Distance, float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Distance = Distance;
        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveStraightDistance, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

// eventuele verbetering : wachten op elkaar met verhogen van setpoint zodat de wielen meer gelijktijdig rijden
void* _DriveStraightDistance(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    float totalangle = _Distance/MMPD;               // total turning angle for the wheels
    float rotspeed = (fabs(_Speed)/100)/MMPD;        // Speed /s --> /10ms is dividing by 100  == [degrees / 10 ms]
    float curangle = 0;
    float curangleHalfDeg = 0;

    // put in PID mode
    LegoMotorSetup(&LegoMotor, MOTOR_R, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_L, 1, 1);

    // reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    if(totalangle > 0)
    {
        // forward
        while(curangle < totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle += rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, curangle*2, KP, KD, KI, IMAX);  // KI and IMAX are 0
                TimeStep(0);
            }
        }
    }
    else if(totalangle < 0)
    {
        // backward
        while(curangle > totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle -= rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, curangle*2, KP, KD, KI, IMAX);  // KI and IMAX are 0
                TimeStep(0);
            }
        }
    }

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Rotate robot around right wheel over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateRWheel(float Angle,float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Angle = Angle;
        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveRotateRWheel, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

void* _DriveRotateRWheel(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    float rotspeed = (fabs(_Speed)/100)/MMPD;    // Speed /s --> /10ms is dividing by 100  == [degrees / 10 ms]
    float totalangle = _Angle*MMRD/MMPD;         // total turning distance for wheel
    float curangle = 0;
    float curangleHalfDeg = 0;

    // enable R/disable L
    LegoMotorSetup(&LegoMotor,MOTOR_R,1,1);
    LegoMotorSetup(&LegoMotor,MOTOR_L,0,0);

    // reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    if(totalangle > 0)
    {
        // forward
        while(curangle < totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle += rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                TimeStep(0);
            }
        }
    }
    else if(totalangle < 0)
    {
        // backward
        while(curangle > totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle -= rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                TimeStep(0);
            }
        }
    }

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Rotate robot around left wheel over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateLWheel(float Angle, float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Angle = Angle;
        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveRotateLWheel, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

void* _DriveRotateLWheel(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    float rotspeed = (fabs(_Speed)/100)/MMPD;    // Speed /s --> /10ms is dividing by 100  == [degrees / 10 ms]
    float totalangle = _Angle*MMRD/MMPD;         // total turning distance for wheel
    float curangle = 0;
    float curangleHalfDeg = 0;

    // enable L/disable R
    LegoMotorSetup(&LegoMotor,MOTOR_L,1,1);
    LegoMotorSetup(&LegoMotor,MOTOR_R,0,0);

    // reset motor angular pos
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    if(totalangle > 0)
    {
        // forward
        while(curangle < totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle += rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                TimeStep(0);
            }
        }
    }
    else if(totalangle < 0)
    {
        // backward
        while(curangle > totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                curangleHalfDeg = curangle * 2;

                _pauseDrivingLoop(&curangleHalfDeg, &curangleHalfDeg);
            }
            else
            {
                curangle -= rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
                TimeStep(0);
            }
        }
    }

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Rotate robot around center of wheelbase over specified angle, at specified speed. Returns after drive is complete.
 * \param Angle :Angle to rotate in deg
 * \param Speed :Speed in mm/s
*/
int DriveRotateCenter(float Angle, float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Angle = Angle;
        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveRotateCenter, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

void* _DriveRotateCenter(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    float rotspeed = (fabs(_Speed)/100)/MMPD;    // Speed /s --> /10ms is dividing by 100  == [degrees / 10 ms]
    float totalangle = _Angle*MMRD/MMPD;         // total turning distance for wheel
    float curangle = 0;
    float complementaryAngle = 0;

    // enable R and L motor
    LegoMotorSetup(&LegoMotor,MOTOR_L,1,1);
    LegoMotorSetup(&LegoMotor,MOTOR_R,1,1);

    // reset pos of wheels
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    if(totalangle > 0)
    {
        while(curangle < totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                complementaryAngle = -curangle;

                _pauseDrivingLoop(&curangle, &complementaryAngle);
            }
            else
            {
                curangle += rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle, KP, KD, KI, IMAX);
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, -curangle, KP, KD, KI, IMAX);
                TimeStep(0);
            }
        }
    }
    else if(totalangle < 0)
    {
        while(curangle > totalangle && !_cancelDriving)
        {
            //Pause drive system if pause flag is high
            if(_pauseDriving)
            {
                complementaryAngle = -curangle;

                _pauseDrivingLoop(&curangle, &complementaryAngle);
            }
            else
            {
                curangle -= rotspeed;
                LegoMotorPIDControl(&LegoMotor,MOTOR_L, curangle, KP, KD, KI, IMAX);
                LegoMotorPIDControl(&LegoMotor,MOTOR_R, -curangle, KP, KD, KI, IMAX);
                TimeStep(0);
            }
        }
    }

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Get 300 sensor samples and calculate the average for each sensor as calibration.
 * \return The average difference between the 2 sensors
*/
int calibrate()
{
    // Delay of 50ms to stabilize after sensor setup in Init function
    _delay_ms(50);

    uint16 iL = 0;      // variable to get left sensor intensity
    uint16 iR = 0;      // variable to get right sensor intensity
    int arr[300];
    int iLarr[300];
    int iRarr[300];
    int i;

    // Get 300 intensities
    for(i = 0; i < 300; i++)
    {
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_L, &iL);
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_R, &iR);
        iLarr[i] = iL;
        iRarr[i] = iR;
        arr[i] = iL-iR;
    }

    int som = 0;
    int somIL = 0;
    int somIR = 0;
    for(i = 0; i < 300; i++)
    {
        som += arr[i];
        somIL += iLarr[i];
        somIR += iRarr[i];
    }
    iLcal = somIL/300;
    iRcal = somIR/300;

    return som/300;
}


/**
 * \brief Follow line until specified distance has been travelled
 * \param Distance :Distance in mm
 * \param Speed :Speed in mm/s
*/
int DriveLineFollowDistance(int Distance, float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Distance = Distance;
        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveLineFollowDistance, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

void* _DriveLineFollowDistance(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    // init motors
    // put in PID mode
    LegoMotorSetup(&LegoMotor,MOTOR_R,1,1);
    LegoMotorSetup(&LegoMotor,MOTOR_L,1,1);

    // reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    // variables
    int totaldist = 0;
    uint16 iL = 0;
    uint16 iR = 0;
    int diff = 0;
    int delta = 0;
    float speedL = 0;
    float speedR = 0;
    float KPlight = 0.01;
    float incL = 0;
    float incR = 0;
    float nextL = 0;
    float nextR = 0;
    float nextLHalfDeg = 0;
    float nextRHalfDeg = 0;
    float rotSpeed = _Speed/100/MMPD;

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    // while distance, nu negeren
    while(totaldist < _Distance && !_cancelDriving)
    {
        // Twee aparte angles ( eentje per wiel )
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_L, &iL);
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_R, &iR);
        diff = iL - iR - cal;

        delta = diff * KPlight;

        // calc speed for both wheels
        if((diff > 1000) || (diff < -1000))
        {
            speedL = _Speed * ( 1 - delta );
            speedR = _Speed * ( 1 + delta );
        }
        else
        {
            speedL = _Speed;
            speedR = _Speed;
        }

        // Check if increase angle is above/under max/min
        incL = speedL/100/MMPD;
        if(incL < 0)
        {
            incL = 0;
        }
        else if (incL > rotSpeed)
        {
            incL = rotSpeed;
        }

        // Check if increase angle is above/under max/min
        incR = speedR/100/MMPD;
        if(incR < 0)
        {
            incR = 0;
        }
        else if (incR > rotSpeed)
        {
            incR = rotSpeed;
        }

        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            nextLHalfDeg = nextL * 2;
            nextRHalfDeg = nextR * 2;

            _pauseDrivingLoop(&nextLHalfDeg, &nextRHalfDeg);
        }
        else
        {
            nextL += incL;
            nextR += incR;

            LegoMotorPIDControl(&LegoMotor,MOTOR_L, (nextL)*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
            LegoMotorPIDControl(&LegoMotor,MOTOR_R, (nextR)*2, KP, KD, KI, IMAX);  // KI and IMAX are 0
            TimeStep(0);

            totaldist = (nextL+nextR)*MMPD /2;
        }
    }

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Follow line until end of line segment
 * \param Speed :Speed in mm/s
*/
int DriveLineFollow(float Speed)
{
    if(!_driveThreadRunning)
    {
        pthread_mutex_lock(&_driveThreadLock);

        _Speed = Speed;

        if(pthread_create(&driveThread, NULL, _DriveLineFollow, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    return 1;
}

void* _DriveLineFollow(void* args)
{
    // Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    // Safety mechanism: speed limited to maximum 100 mm/s.
    if(_Speed > 100)
    {
        _Speed = 100;
    }

    // put in PID mode
    LegoMotorSetup(&LegoMotor,MOTOR_R,1,1);
    LegoMotorSetup(&LegoMotor,MOTOR_L,1,1);

    // reset pos
    LegoMotorSetPos(&LegoMotor, MOTOR_R, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_L, 0);

    // variables
    uint16 iL = 0;
    uint16 iR = 0;
    int diff = 0;
    int delta = 0;
    float speedL = 0;
    float speedR = 0;
    float KPlight = 0.01;
    float incL = 0;
    float incR = 0;
    float nextL = 0;
    float nextR = 0;
    float nextLHalfDeg = 0;
    float nextRHalfDeg = 0;
    float rotSpeed = _Speed/100/MMPD;
    int stop = 0;

    // Setup timestep module with time step of 10ms
    TimeStepInit(10000);

    // while distance, nu negeren
    while(stop == 0 && !_cancelDriving)
    {
        // Twee aparte angles ( eentje per wiel )
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_L, &iL);
        LegoSensorGetLSensorData(&LegoSensor, SENSOR_R, &iR);
        diff = iL - iR - cal;

        delta = diff * KPlight;

        // calc speed for both wheels
        if((diff > 1000) || (diff < -1000))
        {
            speedL = _Speed * ( 1 - delta );
            speedR = _Speed * ( 1 + delta );
        }
        else
        {
            speedL = _Speed;
            speedR = _Speed;
        }

        // Check if increase angle is above/under max/min
        incL = speedL/100/MMPD;
        if(incL < 0)
        {
            incL = 0;
        }
        else if (incL > rotSpeed)
        {
            incL = rotSpeed;
        }

        // Check if increase angle is above/under max/min
        incR = speedR/100/MMPD;
        if(incR < 0)
        {
            incR = 0;
        }
        else if (incR > rotSpeed)
        {
            incR = rotSpeed;
        }

        //Pause drive system if pause flag is hight
        if(_pauseDriving)
        {
            nextLHalfDeg = nextL * 2;
            nextRHalfDeg = nextR * 2;

            _pauseDrivingLoop(&nextLHalfDeg, &nextRHalfDeg);
        }
        else
        {
            nextL += incL;
            nextR += incR;

            LegoMotorPIDControl(&LegoMotor,MOTOR_L, (nextL)*2, KP, KD, KI, IMAX);  // *2 because PID controller works with half degrees
            LegoMotorPIDControl(&LegoMotor,MOTOR_R, (nextR)*2, KP, KD, KI, IMAX);  // KI and IMAX are 0
            TimeStep(0);
        }

        // 8% daling in intensiteit om wit te detecteren
        if((iL < (iLcal*0.92)) && (iR < (iRcal*0.92)))
        {
            stop = 1;
        }
    }

    _driveThreadRunning = false;

    return NULL;
}

void _pauseDrivingLoop(float* lMotorAngle, float* rMotorAngle)
{
    while(1)
    {
        if(_pauseDriving && !_cancelDriving)
        {
            //Turn motorspeed to 0
            LegoMotorPIDControl(&LegoMotor,MOTOR_L, *lMotorAngle, KP, KD, KI, IMAX);
            LegoMotorPIDControl(&LegoMotor,MOTOR_R, *rMotorAngle, KP, KD, KI, IMAX);
        }
        else
        {
            //Exit loop
            return;
        }
    }
}
