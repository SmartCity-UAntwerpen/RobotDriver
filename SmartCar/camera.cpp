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
    if(cap.isOpened())
    {
        //Camera already open
        return 1;
    }

    //Open camera with param index: cam
    cap.open(cam);

    if(!cap.isOpened())
    {
        //Camera will not open (camera not connected?)
        return 2;
    }

    //Set camera frame size
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 1024);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 786);

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
