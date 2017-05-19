#pragma once
#include <qimage.h>
#include <cv.h>
using namespace cv;
QImage cvMat2QImage(const cv::Mat& mat);
cv::Mat QImage2cvMat(QImage image);