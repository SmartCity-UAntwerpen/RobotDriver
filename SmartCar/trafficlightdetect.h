#ifndef TRAFFICLIGHTDETECT_H
#define TRAFFICLIGHTDETECT_H

#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

cv::Mat rotate(cv::Mat src, double angle);

cv::Mat HSVmergedImage(cv::Mat im, cv::Mat hsv, cv::Mat binary, cv::Mat binary1, cv::Mat binary2, cv::Mat tssImage);

cv::Mat colorBoundry(cv::Mat im, int areaTreshold, bool green, bool red);

#endif
