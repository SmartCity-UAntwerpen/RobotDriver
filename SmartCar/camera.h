#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

#define CAMERA_CAPTURE_W 640
#define CAMERA_CAPTURE_H 480

int initCamera(void);

int initCamera(int cam);

int closeCamera(void);

cv::VideoCapture getCameraCapture(void);

#endif
