#ifndef DETECT_CAMERA_ERRORS_H
#define DETECT_CAMERA_ERRORS_H
#include <qthread.h>
#include <opencv2/opencv.hpp>
#include "../include/QtGigE/qtgige.h"


class error_detector : public QThread
{
    Q_OBJECT
public:
    error_detector(void);
    ~error_detector();
public slots:
    void newBayerGRImage(cv::Mat img, qint64 timestamp);
signals:
    void newImage(cv::Mat img, qint64 timestamp);
private:
    cv::Mat previousImage;
    cv::Mat tempImage;
    int errorCounter;
};

#endif