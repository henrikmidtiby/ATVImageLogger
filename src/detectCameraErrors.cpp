#include "detectCameraErrors.h"

#include "detectCameraErrors.moc"
error_detector::error_detector(void )
{
    errorCounter = 0;
    std::cout << "previousImage.rows = " << previousImage.rows << std::endl;
}

error_detector::~error_detector()
{
}

void error_detector::newBayerGRImage(cv::Mat img, qint64 timestamp)
{
//     std::cout << "<error_detector::newBayerGRImage />" << std::endl;
    cv::Mat BayerGR8;
    QTGIGE::convert16to8bit(img, BayerGR8);
    if(previousImage.rows > 0) 
    {
  	tempImage = 10*abs(BayerGR8 - previousImage) + 10*abs(previousImage - BayerGR8);
	emit(newImage(tempImage, timestamp));    
    }
    else
    {
	std::cout << "Creating previousImage and tempImage." << std::endl;
    }
    previousImage = BayerGR8.clone();
}

