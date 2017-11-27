#include "camera.h"

using namespace cv;

VideoCapture cap;

int initCamera()
{
    //Open default camera
    return initCamera(0);
}

int initCamera(int cam)
{
    FILE* modulesGrep;
    char buffer[3];

    if(cap.isOpened())
    {
        //Camera already open
        return 1;
    }

    //Check if V4L2-drivers are installed into Linux kernel
    //Pipe the grep search for the V4L2-driver module
    modulesGrep = popen("cat /proc/modules | if grep -q bcm2835_v4l2; then echo 1; else echo 0; fi;", "r");
    if(!modulesGrep)
    {
        printf("Error, cannot open driver search command\n");
        return 3;   //Other error
    }

    bool moduleLoaded = false;

    while(fgets(buffer, 3, modulesGrep) != NULL)
    {
        //Check for status flag
        if(strcmp(buffer, "1") == 0)
        {
            moduleLoaded = true;
        }
    }

    if(!moduleLoaded)
    {
        //Try to install the V4L2-drivers into Linux kernel
        system("sudo modprobe bcm2835-v4l2 2>/dev/null");
    }

    //Open camera with param index: cam
    cap.open(cam);

    if(!cap.isOpened())
    {
        //Camera will not open (camera not connected?)
        return 2;
    }

    //Set camera frame size
    cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_CAPTURE_W);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_CAPTURE_H);
    cap.set(CV_CAP_PROP_FPS, 60);

    return 0;
}

int closeCamera()
{
    if(!cap.isOpened())
    {
        //Camera already closed
        return 1;
    }

    //Close camera stream
    cap.release();

    if(cap.isOpened())
    {
        //Camera will not close
        return 2;
    }

    return 0;
}

VideoCapture getCameraCapture()
{
    return cap;
}
