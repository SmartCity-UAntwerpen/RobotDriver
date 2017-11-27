/**
    Code developed by Quinten Van Hasselt & Timothy Verstraete
    Modified by Thomas Huybrechts
**/

#include "lift.h"

static bool liftInit = false;

int LiftInit(void)
{
    //Lift setup (no brake, direct control mode)
    LegoMotorSetup(&LegoMotor, 3, 0, 0);
    LegoMotorDirectControl(&LegoMotor, 3, 0);

    //Sensor setup for lift switch
    LegoSensorSetup(&LegoSensor, 3, CFG_SWITCH);

    //Init switchstate as if pressed
    uint8 switchState = 1;

    if(LegoSensorGetSwitchData(&LegoSensor, 3, &switchState) == 0)
    {
        //Switch not pressed, go down:
        if(switchState == 0)
        {
            //Negative PWM to move down.
            LegoMotorDirectControl(&LegoMotor, 3, -30000);

            //Keep going down until switchstate != 0
            while(switchState == 0)
            {
                //Get new switchstate
                LegoSensorGetSwitchData(&LegoSensor, 3, &switchState);
            }

            //Reset PWM to 0
            LegoMotorDirectControl(&LegoMotor, 3, 0);
        }
    }
    else
    {
        //Could not get state of switch
        //Turn off motor
        LegoMotorDirectControl(&LegoMotor, 3, 0);

        //Put brake on lift
        LegoMotorSetup(&LegoMotor, 3, 1, 0);

        liftInit = false;

        return 1;
    }

    //Put brake on lift
    LegoMotorSetup(&LegoMotor, 3, 1, 0);

    _delay_ms(200);

    //Set angle as 0
    LegoMotorSetPos(&LegoMotor, 3, 0);

    liftInit = true;

    return 0;
}

int LiftGoto(float Height)
{
    if(!liftInit)
    {
        //Lift is not initialised
        return 1;
    }

    if(Height < 0)
    {
        //Negative heights are not allowed
        Height = 0;
    }

    //Check for max Height of MAX_LIFTHEIGHT mm
    if(Height > MAX_LIFTHEIGHT)
    {
        Height = MAX_LIFTHEIGHT;
    }

    //Get current height
    float currentHeight;
    LiftGetHeight(&currentHeight);

    if((Height >= 0) && (Height != currentHeight))
    {
        //Get current angular motor position
        sint16 pos;
        LegoMotorGetPos(&LegoMotor, 3, &pos);

        //Calculate new position:
        float diff = Height-currentHeight;
        sint16 newPos = pos + (diff*81.82);

        //Go UP
        if(Height > currentHeight)
        {
            //Go UP
            LegoMotorDirectControl(&LegoMotor, 3, 30000);

            while(pos <= newPos)
            {
                LegoMotorGetPos(&LegoMotor, 3, &pos);
            }

            LegoMotorDirectControl(&LegoMotor, 3, 0);
        }
        else
        {
            //Go DOWN
            LegoMotorDirectControl(&LegoMotor, 3, -30000);

            while(pos >= newPos)
            {
                LegoMotorGetPos(&LegoMotor, 3, &pos);
            }

            LegoMotorDirectControl(&LegoMotor, 3, 0);
        }
    }

    return 0;
}

int LiftGetHeight(float *Height)
{
    if(Height != NULL)
    {
        sint16 pos = 0;
        if(LegoMotorGetPos(&LegoMotor, 3, &pos) == 0)
        {
            *Height = pos / 81.82;

            return 0;
        }
        else
        {
            //Error while retrieving lift height
            return 2;
        }
    }
    else
    {
        //Pointer can not be NULL
        return 1;
    }
}

bool liftInitialised(void)
{
    return liftInit;
}
