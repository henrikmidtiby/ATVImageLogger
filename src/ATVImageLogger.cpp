#include "ATVImageLogger.h"
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QApplication>
#include <QTimer>

#define BUFFER_LENGTH 100

ATVImageLogger::ATVImageLogger()
{
    qRegisterMetaType< cv::Mat >("cv::Mat");

    startCameras();
    startLoggingSystem();

    drawGui();
    connectImageProducersToGui();
    connectGuiElements();
}

void ATVImageLogger::startCameras()
{
    this->cameraOne = startCamera("Basler-21322519");
    // This delay is needed, otherwise the cameras seems to interfere with each other.
    usleep(10);
    this->cameraTwo = startCamera("Basler-21325585");
}

QTGIGE * ATVImageLogger::startCamera(char* deviceId)
{
    LoggerModule cameraInfo("../Logging", "Camera information");
    char buffer[BUFFER_LENGTH];

    // Initialize cameras
    QTGIGE * device = new QTGIGE(deviceId);
    std::cout << "device: " << device << std::endl;
    snprintf(buffer, BUFFER_LENGTH, "Opening device: %s", deviceId);
    cameraInfo.log("startCamera", buffer);
    
    // Set ROI
    int width = 800;
    int height = 800;
    int offsetWidth = 600;
    int offsetHeight = 0;
    snprintf(buffer, BUFFER_LENGTH, "Setting roi: %d %d %d %d", width, height, offsetWidth, offsetHeight);
    cameraInfo.log("startCamera", buffer);
    device->setROI(offsetWidth, offsetHeight, width, height);
    

    // Set image acquisition rate
    int acqFramerate = 10;
    device->writeBool("AcquisitionFrameRateEnable", true);
    device->writeFloat("AcquisitionFrameRateAbs", 10);
    snprintf(buffer, BUFFER_LENGTH, "Setting framerate: %d", acqFramerate);
    cameraInfo.log("startCamera", buffer);

    // Set exposure time
    int rawExposureTime = 3000;
    device->writeInt("ExposureTimeRaw", 3000);
    snprintf(buffer, BUFFER_LENGTH, "Setting exposure time: %d", rawExposureTime);
    cameraInfo.log("startCamera", buffer);
    
    // Start image acquisition
    device->startAquisition();
    
    return device;
}

void ATVImageLogger::startLoggingSystem()
{
    // Initialize logger modules
    // Henrik: If the following line is not active a linker error is thrown ...
    LoggerModule test("../Logging", "Tester");
    this->loggerOne = new ImageLogger("../Logging", "CameraOne");
    this->loggerTwo = new ImageLogger("../Logging", "CameraTwo");
    
    // Connect cameras with the logging system
    connect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), loggerOne, SLOT(burstImageLogger(cv::Mat, qint64)), Qt::QueuedConnection);
    connect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), loggerTwo, SLOT(burstImageLogger(cv::Mat, qint64)), Qt::QueuedConnection);
}

void ATVImageLogger::drawGui(void )
{
    // Create camera views
    this->viewOne = new CQtOpenCVViewerGl(this);
    this->viewTwo = new CQtOpenCVViewerGl(this);
    this->viewOne->setMinimumHeight(400);
    this->viewOne->setMinimumWidth(1024);
    this->viewTwo->setMinimumHeight(400);
    this->viewTwo->setMinimumWidth(1024);
    
    // Create button for activating burst loggers
    this->LogBurstBtn = new QPushButton("Log image burst");
    connect(this->LogBurstBtn, SIGNAL(pressed()), loggerOne, SLOT(saveImageBurst()));
    connect(this->LogBurstBtn, SIGNAL(pressed()), loggerTwo, SLOT(saveImageBurst()));

    // Define global widget
    this->globalWidget = new QWidget(this);
    
    // Create combobox for selecting which image to show
    this->imageSelect = new QComboBox(globalWidget);
    //connect(this->imageSelect, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentViewChanged(QString)));
    connect(this->imageSelect, SIGNAL(currentIndexChanged(QString)), this, SLOT(imshowSelectorChanged(QString)));
    this->imageSelect->addItem("Input");
    this->imageSelect->addItem("Excess Green");

    // Create other gui elements
    this->cameraSettingsOneBtn = new QPushButton("Camera one settings");
    this->cameraSettingsTwoBtn = new QPushButton("Camera two settings");
    this->modicoviText = new QLabel("Modicovi Score:");
    
    // Define placement of gui elements
    this->Layout = new QGridLayout(this->globalWidget);
    this->sideWidget = new QWidget(globalWidget);
    this->sideLayout = new QGridLayout(this->sideWidget);
    this->Layout->addWidget(viewOne, 1,1);
    this->Layout->addWidget(viewTwo, 2,1);
    this->Layout->addWidget(imageSelect, 3,1);
    this->Layout->addWidget(sideWidget, 1,2);
    this->sideLayout->addWidget(modicoviText, 1,1);
    this->sideLayout->addWidget(LogBurstBtn, 2,1);
    this->sideLayout->addWidget(cameraSettingsOneBtn, 3,1);
    this->sideLayout->addWidget(cameraSettingsTwoBtn, 4,1);
    this->sideWidget->setLayout(this->sideLayout);
    this->globalWidget->setLayout(this->Layout);
    setCentralWidget(this->globalWidget);
}

void ATVImageLogger::connectImageProducersToGui()
{
    connect(&exgOne, SIGNAL(newImage(cv::Mat, qint64)), viewOne, SLOT(showImage(cv::Mat, qint64)));
    connect(&exgTwo, SIGNAL(newImage(cv::Mat, qint64)), viewTwo, SLOT(showImage(cv::Mat, qint64)));
    connect(&demOne, SIGNAL(newImage(cv::Mat, qint64)), viewOne, SLOT(showImage(cv::Mat, qint64)));
    connect(&demTwo, SIGNAL(newImage(cv::Mat, qint64)), viewTwo, SLOT(showImage(cv::Mat, qint64)));
}

void ATVImageLogger::connectGuiElements()
{
    connect(cameraSettingsOneBtn, SIGNAL(pressed()), cameraOne, SLOT(showCameraSettings()));
    connect(cameraSettingsTwoBtn, SIGNAL(pressed()), cameraTwo, SLOT(showCameraSettings()));
}

void ATVImageLogger::currentViewChanged(const QString& text)
{
    std::cout << "Received new view " << text.toLocal8Bit().data() << std::endl;
    disconnect(this->viewOne, SLOT(showImage(cv::Mat*)));
}

void ATVImageLogger::imshowSelectorChanged(QString text)
{
    std::cout << "<imshowSelectorChanged(" << text.toLocal8Bit().data() << ")>" << std::endl;
    disconnect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &demOne, SLOT(newBayerGRImage(cv::Mat, qint64)));
    disconnect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &demTwo, SLOT(newBayerGRImage(cv::Mat, qint64)));
    disconnect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgOne, SLOT(newBayerGRImage(cv::Mat, qint64)));
    disconnect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgTwo, SLOT(newBayerGRImage(cv::Mat, qint64)));

    if(text.contains("Input"))
    {
        connect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &demOne, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);
        connect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &demTwo, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);
    }
    if(text.contains("Excess Green"))
    {
	connect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgOne, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);
	connect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgTwo, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);
    }
}

ATVImageLogger::~ATVImageLogger()
{}

#include "ATVImageLogger.moc"
