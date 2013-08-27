#include "detectCameraErrors.h"

#include "detectCameraErrors.moc"
error_detector::error_detector(void )
{
    errorCounter = 0;
}

error_detector::~error_detector()
{
}

void error_detector::newBayerGRImage(cv::Mat img, qint64 timestamp)
{
    // Method for comparing timewise adjacent images and spot similar regions in these images.
    cv::Mat BayerGR8;
    QTGIGE::convert16to8bit(img, BayerGR8);
    if(previousImage.rows > 0) 
    {
	// An earlier image was stored, so calculate the absolute difference and 
	// emit a signal with the difference image.
	tempImage = 10*abs(BayerGR8 - previousImage) + 10*abs(previousImage - BayerGR8);
	emit(newImage(tempImage, timestamp));    
    }
    previousImage = BayerGR8.clone();
}

