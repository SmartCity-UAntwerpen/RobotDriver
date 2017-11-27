#include "drive.h"
#include "math.h"
#include "timestep.h"

static pthread_t driveThread;
static pthread_mutex_t _driveThreadLock;

static float const wheelAnglePerMM = 2.18181818182;
static float const driveWheelBaseMM = 115.0;
static uint16 const motor_KP = 1800;   // first value was 2100, this one seems more stable
static uint16 const motor_KD = 4200;   // first value was 5000, this one seems more stable
static uint16 const motor_KI = 0;
static uint16 const motor_IMAX = 0;
static float const line_KP = 300E-6;  //KP value for differential calculation of the line follower
static int const MAX_SPEED = 80;      //Max motor speed

uint16 sensorLeftCal = 0;              //Left sensor calibrated value on drive stroke
uint16 sensorRightCal = 0;             //Right sensor calibrated value on drive stroke
sint16 sensorOffset = 0;               //Difference between right light sensor and left light sensor
sint16 encoderL = 0;                   //Left wheel encoder position
sint16 encoderR = 0;                   //Right wheel encoder position
int overflowsEncoderL = 0;             //Number of overflows on L encoders
int overflowsEncoderR = 0;             //Number of overflows on R encoders

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
    error += LegoMotorSetup(&LegoMotor,MOTOR_RIGHT,0,0);
    error += LegoMotorSetup(&LegoMotor,MOTOR_LEFT,0,0);
    error += LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0);
    error += LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);
    error += LegoMotorPosPIDControl(&LegoMotor, MOTOR_RIGHT, 0);
    error += LegoMotorPosPIDControl(&LegoMotor, MOTOR_LEFT, 0);
    error += LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    error += LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);

    //Setup reflection sensors
    error += LegoSensorSetup(&LegoSensor, LSENSOR_RIGHT, CFG_LSENSOR);
    error += LegoSensorSetup(&LegoSensor, LSENSOR_LEFT, CFG_LSENSOR);
    error += LegoSensorSetupLSensor(&LegoSensor, LSENSOR_RIGHT, 1);
    error += LegoSensorSetupLSensor(&LegoSensor, LSENSOR_LEFT, 1);

    //Calibrate
    error += Calibrate();

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

        //Reset PID-drivers
        LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
        LegoMotorPIDControl(&LegoMotor, MOTOR_LEFT, 0, motor_KP, motor_KD, motor_KI, motor_IMAX);
        LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
        LegoMotorPIDControl(&LegoMotor, MOTOR_RIGHT, 0, motor_KP, motor_KD, motor_KI, motor_IMAX);

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
    //Calculate position in mm
    if(overflowsEncoderL >= 0)
    {
        *posL = ((encoderL + ENCODER_MAX_POS_VAL * overflowsEncoderL) / wheelAnglePerMM)/2;
    }
    else
    {
        *posL = ((encoderL + ENCODER_MAX_NEG_VAL * overflowsEncoderL) / wheelAnglePerMM)/2;
    }

    if(overflowsEncoderR >= 0)
    {
        *posR = ((encoderR + ENCODER_MAX_POS_VAL * overflowsEncoderR) / wheelAnglePerMM)/2;
    }
    else
    {
        *posR = ((encoderR + ENCODER_MAX_NEG_VAL * overflowsEncoderR) / wheelAnglePerMM)/2;
    }

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
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(Distance == 0)
    {
        //Reset angular motor position
        encoderL = 0;
        encoderR = 0;
        overflowsEncoderL = 0;
        overflowsEncoderR = 0;

        _driveThreadRunning = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveStraightDistance(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    float curAngle = 0;
    float deltaTarget = (fabs(_Speed) * wheelAnglePerMM) / 100 * ((_Distance < 0) ? -1:1);      //Set delta step to regulate driving speed and rotation direction
    sint32 targetAngle = (sint32)(_Distance * wheelAnglePerMM);

    //Put in PID mode
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 1);

    //Reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Drive loop
    while(fabs(curAngle) < fabs(targetAngle) && !_cancelDriving)
    {
        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            curAngle += deltaTarget;

            setLeftMotorEncoder(curAngle);
            setRightMotorEncoder(curAngle);

            TimeStep(0);
        }
    }

    //Turn of motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

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
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(Angle == 0)
    {
        //Reset angular motor position
        encoderL = 0;
        encoderR = 0;
        overflowsEncoderL = 0;
        overflowsEncoderR = 0;

        _driveThreadRunning = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveRotateRWheel(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    float curAngle = 0;
    float nullAngle = 0;
    float deltaTarget = (_Speed * wheelAnglePerMM) / 100 * ((_Angle < 0) ? -1:1);   //Set delta step to regulate rotation speed and turn direction
    sint32 targetAngle = (sint32)(wheelAnglePerMM * 2 * driveWheelBaseMM * 3.14159 / 360 * _Angle);

    //Enable R/disable L
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 0, 1);

    //Reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Turning with right wheel
    while(fabs(curAngle) < fabs(targetAngle) && !_cancelDriving)
    {
        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            curAngle += deltaTarget;

            setLeftMotorEncoder(nullAngle);
            setRightMotorEncoder(curAngle);

            TimeStep(0);
        }
    }

    //Turn of motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

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
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(Angle == 0)
    {
        //Reset angular motor position
        encoderL = 0;
        encoderR = 0;
        overflowsEncoderL = 0;
        overflowsEncoderR = 0;

        _driveThreadRunning = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveRotateLWheel(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    float curAngle = 0;
    float nullAngle = 0;
    float deltaTarget = (_Speed * wheelAnglePerMM) / 100 * ((_Angle < 0) ? -1:1);   //Set delta step to regulate rotation speed and turn direction
    sint32 targetAngle = (sint32)(wheelAnglePerMM * 2 * driveWheelBaseMM * 3.14159 / 360 * _Angle);

    //Enable L/disable R
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 0, 1);

    //Reset motor angular position
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Turning with left wheel
    while(fabs(curAngle) < fabs(targetAngle) && !_cancelDriving)
    {
        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            curAngle += deltaTarget;

            setLeftMotorEncoder(curAngle);
            setRightMotorEncoder(nullAngle);

            TimeStep(0);
        }
    }

    //Turn of motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

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
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(Angle == 0)
    {
        //Reset angular motor position
        encoderL = 0;
        encoderR = 0;
        overflowsEncoderL = 0;
        overflowsEncoderR = 0;

        _driveThreadRunning = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveRotateCenter(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    float curAngleLeft = 0;
    float curAngleRight = 0;
    float deltaTarget = (_Speed * wheelAnglePerMM) / 100 * ((_Angle < 0) ? -1:1); //Set delta step to regulate rotation speed and turn direction
    sint32 targetAngle = (sint32)(wheelAnglePerMM * driveWheelBaseMM * 3.14159 / 360 * _Angle);

    //Enable R and L motor
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 1);

    //Reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Rotating
    while((fabs(curAngleRight) + fabs(curAngleLeft)) / 2 < fabs(targetAngle) && !_cancelDriving)
    {
        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            curAngleLeft += -deltaTarget;
            curAngleRight += deltaTarget;

            setLeftMotorEncoder(curAngleLeft);
            setRightMotorEncoder(curAngleRight);

            TimeStep(0);
        }
    }

    //Turn of motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Get 300 sensor samples and calculate the average for each sensor as calibration.
 * \return The average difference between the 2 sensors
*/
int Calibrate(void)
{
    pthread_mutex_lock(&_driveThreadLock);

    if(!_driveThreadRunning)
    {
        if(pthread_create(&driveThread, NULL, _Calibrate, NULL) < 0)
        {
            printf("Error while creating drive thread!\n");

            pthread_mutex_unlock(&_driveThreadLock);

            return 2;
        }

        _driveThreadRunning = true;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _Calibrate(void* args)
{
    //Delay of 500ms to stabilize after sensor setup in Init function
    _delay_ms(500);

    uint16 iL = 0;      //Variable to store left sensor intensity
    uint16 iR = 0;      //Variable to store right sensor intensity
    uint16 iLarr[300];
    uint16 iRarr[300];
    sint16 arr[300];
    int i;

    //Get 300 intensities
    for(i = 0; i < 300; i++)
    {
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_LEFT, &iL);
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_RIGHT, &iR);
        iLarr[i] = iL;
        iRarr[i] = iR;
        arr[i] = (sint16)iL - (sint16)iR;
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

    sensorLeftCal = somIL/300;
    sensorRightCal = somIR/300;
    sensorOffset = som/300;

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Follow line until specified distance has been traveled
 * \param Distance :Distance in mm
 * \param Speed :Speed in mm/s
*/
int DriveLineFollowDistance(int Distance, float Speed)
{
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(Distance == 0)
    {
        //Reset angular motor position
        encoderL = 0;
        encoderR = 0;
        overflowsEncoderL = 0;
        overflowsEncoderR = 0;

        _driveThreadRunning = false;

        pthread_mutex_unlock(&_driveThreadLock);

        return 0;
    }

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveLineFollowDistance(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    //Variables
    float diff;
    float speedL = 0, speedR = 0;
    float currAngleLeft = 0, currAngleRight = 0;
    float deltaTargetLeft = 0, deltaTargetRight = 0;
    uint16 sensorDataLeft, sensorDataRight;
    sint32 targetAngle = (sint32)(_Distance * wheelAnglePerMM);

    //Put in PID mode
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 1);

    //Reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Drive follow line loop
    while((fabs(currAngleLeft) + fabs(currAngleRight)) / 2 < fabs(targetAngle) && !_cancelDriving)
    {
        //Get light sensor data
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_LEFT, &sensorDataLeft);
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_RIGHT, &sensorDataRight);
        sensorDataRight += sensorOffset;

        //Calculate differential steering
        diff = (float)sensorDataRight - (float)sensorDataLeft;

        //Multiply an additional KP factor for the line follower
        diff *= line_KP;

        //Calculate speed for each wheel
        speedL = _Speed * (1 + diff);
        speedR = _Speed * (1 - diff);

        //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
        if(fabs(speedL) > MAX_SPEED)
        {
            _Speed = MAX_SPEED * (speedL < 0 ? -1 : 1);
        }

        if(fabs(speedR) > MAX_SPEED)
        {
            _Speed = MAX_SPEED * (speedR < 0 ? -1 : 1);
        }

        //Calculate wheel speeds
        deltaTargetLeft = (speedL * wheelAnglePerMM) / 100;
        deltaTargetRight = (speedR * wheelAnglePerMM) / 100;

        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            currAngleLeft += deltaTargetLeft;
            currAngleRight += deltaTargetRight;

            setLeftMotorEncoder(currAngleLeft);
            setRightMotorEncoder(currAngleRight);

            TimeStep(0);
        }
    }

    //Turn of motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

    _driveThreadRunning = false;

    return NULL;
}


/**
 * \brief Follow line until end of line segment
 * \param Speed :Speed in mm/s
*/
int DriveLineFollow(float Speed)
{
    pthread_mutex_lock(&_driveThreadLock);

    _cancelDriving = false;

    if(!_driveThreadRunning)
    {
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

    pthread_mutex_unlock(&_driveThreadLock);

    return 1;
}

void* _DriveLineFollow(void* args)
{
    //Delay of 50ms to stabilize sensors and motors
    _delay_ms(50);

    //Safety mechanism: speed limited to maximum MAX_SPEED mm/s.
    if(fabs(_Speed) > MAX_SPEED)
    {
        _Speed = MAX_SPEED;
    }

    //Variables
    float diff;
    float speedL = 0, speedR = 0;
    float targetAngleLeft = 0, targetAngleRight = 0;
    float deltaTargetLeft = 0, deltaTargetRight = 0;
    char stop = 0;
    uint16 sensorDataLeft, sensorDataRight;

    //Put in PID mode
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 1);
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 1);

    //Reset angular motor position
    LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
    LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
    encoderL = 0;
    encoderR = 0;
    overflowsEncoderL = 0;
    overflowsEncoderR = 0;

    //Setup timestep module with time step of 10ms
    TimeStepInit(TIMESLOT_LEN);

    //Drive follow line loop
    while(stop == 0 && !_cancelDriving)
    {
        //Get light sensor data
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_LEFT, &sensorDataLeft);
        LegoSensorGetLSensorData(&LegoSensor, LSENSOR_RIGHT, &sensorDataRight);
        sensorDataRight += sensorOffset;

        //Calculate differential steering
        diff = (float)sensorDataRight - (float)sensorDataLeft;

        //Multiply an additional KP factor for the line follower
        diff *= line_KP;

        //Calculate speed for each wheel
        speedL = _Speed * (1 + diff);
        speedR = _Speed * (1 - diff);

        if(fabs(speedL) > MAX_SPEED)
        {
            speedL = MAX_SPEED * (speedL < 0 ? -1 : 1);
        }

        if(fabs(speedR) > MAX_SPEED)
        {
            speedR = MAX_SPEED * (speedR < 0 ? -1 : 1);
        }

        //Calculate wheel speeds
        deltaTargetLeft = (speedL * wheelAnglePerMM) / 100;
        deltaTargetRight = (speedR * wheelAnglePerMM) / 100;

        //Pause drive system if pause flag is high
        if(_pauseDriving)
        {
            _pauseDrivingLoop(&encoderL, &encoderR);
        }
        else
        {
            targetAngleLeft += deltaTargetLeft;
            targetAngleRight += deltaTargetRight;

            setLeftMotorEncoder(targetAngleLeft);
            setRightMotorEncoder(targetAngleRight);

            TimeStep(0);
        }

        // 8% drop in intensity to detect white
        if((sensorDataLeft < (sensorLeftCal*0.92)) && (sensorDataRight < (sensorRightCal*0.92) + sensorOffset))
        {
            stop = 1;
        }
    }

    //Turn off motors with brakes
    LegoMotorSetup(&LegoMotor, MOTOR_RIGHT, 1, 0);      //Set motor brake
    LegoMotorSetup(&LegoMotor, MOTOR_LEFT, 1, 0);
    LegoMotorDirectControl(&LegoMotor, MOTOR_RIGHT, 0); //Turn off motor (PWM: 0)
    LegoMotorDirectControl(&LegoMotor, MOTOR_LEFT, 0);

    _driveThreadRunning = false;

    return NULL;
}

void _pauseDrivingLoop(sint16* lMotorAngle, sint16* rMotorAngle)
{
    while(1)
    {
        if(_pauseDriving && !_cancelDriving)
        {
            //Turn motorspeed to 0
            LegoMotorPIDControl(&LegoMotor, MOTOR_LEFT, *lMotorAngle, motor_KP, motor_KD, motor_KI, motor_IMAX);
            LegoMotorPIDControl(&LegoMotor, MOTOR_RIGHT, *rMotorAngle, motor_KP, motor_KD, motor_KI, motor_IMAX);
        }
        else
        {
            //Exit loop
            return;
        }

        encoderL = *lMotorAngle;
        encoderR = *rMotorAngle;
    }
}

void setLeftMotorEncoder(float angle)
{
    if(angle >= 0)
    {
        //Check for encoder overflow
        //Encoder works with half degrees
        if(angle*2 >= ENCODER_MAX_POS_VAL * (overflowsEncoderL + 1))
        {
            overflowsEncoderL++;
            LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
        }

        encoderL = angle * 2 - ENCODER_MAX_POS_VAL * overflowsEncoderL;
    }
    else
    {
        //Check for encoder overflow
        //Encoder works with half degrees
        if(angle*2 <= ENCODER_MAX_NEG_VAL * (overflowsEncoderL - 1))
        {
            overflowsEncoderL--;
            LegoMotorSetPos(&LegoMotor, MOTOR_LEFT, 0);
        }

        encoderL = angle * 2 - ENCODER_MAX_NEG_VAL * overflowsEncoderL;
    }

    LegoMotorPIDControl(&LegoMotor, MOTOR_LEFT, encoderL, motor_KP, motor_KD, motor_KI, motor_IMAX);  // *2 because PID controller works with half degrees (KI and IMAX are 0)
}

void setRightMotorEncoder(float angle)
{
    if(angle >= 0)
    {
        //Check for encoder overflow
        //Encoder works with half degrees
        if(angle*2 >= ENCODER_MAX_POS_VAL * (overflowsEncoderR + 1))
        {
            overflowsEncoderR++;
            LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
        }

        encoderR = angle * 2 - ENCODER_MAX_POS_VAL * overflowsEncoderR;
    }
    else
    {
        //Check for encoder overflow
        //Encoder works with half degrees
        if(angle*2 <= ENCODER_MAX_NEG_VAL * (overflowsEncoderR - 1))
        {
            overflowsEncoderR--;
            LegoMotorSetPos(&LegoMotor, MOTOR_RIGHT, 0);
        }

        encoderR = angle * 2 - ENCODER_MAX_NEG_VAL * overflowsEncoderR;
    }

    LegoMotorPIDControl(&LegoMotor, MOTOR_RIGHT, encoderR, motor_KP, motor_KD, motor_KI, motor_IMAX);  // *2 because PID controller works with half degrees (KI and IMAX are 0)
}
