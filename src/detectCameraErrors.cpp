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
	std::cout << "Calculating image differences." << std::endl;
	// Previous image is defined
 	tempImage = BayerGR8 - previousImage;
// 	tempImage = abs(tempImage);
	
	
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
        QString targetFilename = "Testing.png";
        cv::imwrite(targetFilename.toStdString(), BayerGR8, compression_params);
	
	cv::Mat rowSums;
	std::cout << "3" << std::endl;
	cvReduce(&tempImage, &rowSums, 1);
	std::cout << "4" << std::endl;
	
    }
    else
    {
	std::cout << "Creating previousImage and tempImage." << std::endl;
	tempImage = BayerGR8.clone();
    }
    previousImage = BayerGR8.clone();
}

