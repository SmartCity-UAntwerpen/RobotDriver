#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

int initCamera(void);

int initCamera(int cam);

int closeCamera(void);

cv::VideoCapture getCameraCapture(void);

#endif
