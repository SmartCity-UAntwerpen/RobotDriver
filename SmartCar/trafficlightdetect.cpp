#include "trafficlightdetect.h"

using namespace cv;
using namespace std;

RNG rng(12345);

Mat rotate(Mat src, double angle)
{
    Mat dst;
    Point2f pt(src.cols/2., src.rows/2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine(src, dst, r, Size(src.cols, src.rows));

    return dst;
}

Mat HSVmergedImage(Mat im, Mat hsv, Mat binary, Mat binary1, Mat binary2, Mat tssImage)
{
    Mat imgToProcess;

    //To HSV
    cvtColor(im, hsv, CV_BGR2HSV);

    //Get binary image
    inRange(hsv, Scalar(90, 20, 0), Scalar(120, 255, 255), binary);     //Blue
    inRange(hsv, Scalar(56, 240, 100), Scalar(70, 255, 255), binary1);  //Green
    inRange(hsv, Scalar(170, 215, 50), Scalar(180, 255, 200), binary2); //Red

    //Merge images
    add(binary, binary1, tssImage);
    add(binary2, tssImage, imgToProcess);

    return imgToProcess;
}

Mat colorBoundry(Mat im, int areaTreshold, bool green, bool red)
{
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(im, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));   //Find contours

    vector< vector<Point> > contours_poly(contours.size());
    vector<Rect> boundRect(contours.size());
    vector<float> area(contours.size());
    vector<Point2f> center(contours.size());
    vector<Moments> mu(contours.size());

    Mat drawing = Mat::zeros(im.size(), CV_8UC3);

    for(size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
        boundRect[i] = boundingRect(Mat(contours_poly[i]));
        area[i] = contourArea(Mat(contours_poly[i]));
    }

    //Get the moments
    for(size_t i = 0; i < contours.size(); i++)
    {
        mu[i] = moments(contours[i], false);
    }

    int contourInt = 0;

    for(size_t i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

        if(area[i] > areaTreshold)
        {
            rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);

            center[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);

            if(contourInt != 0)
            {
                if((boundRect[i].tl().x >= boundRect[contourInt].tl().x) && (boundRect[i].br().x <= boundRect[i].br().x))
                {
                    if(center[i].y <= center[contourInt].y)
                    {
                        green = true;
                        red = false;
                    }
                    else if(center[i].y >= center[contourInt].y)
                    {
                        red = true;
                        green = false;
                    }
                }
            }

            contourInt = i;
        }
    }

    return drawing;
}


