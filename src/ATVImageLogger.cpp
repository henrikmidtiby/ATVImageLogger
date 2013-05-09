#include "ATVImageLogger.h"
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QApplication>
#include <QTimer>

ATVImageLogger::ATVImageLogger()
{
    qRegisterMetaType< cv::Mat >("cv::Mat");
    
    // Initialize cameras
    this->cameraOne = new QTGIGE("Basler-21272795");
    this->cameraTwo = new QTGIGE("Basler-21272796");
    
    // Start image acquisition
    this->cameraOne->startAquisition();
    this->cameraTwo->startAquisition();
    
    // Connect cameras with the gui
    connect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgOne, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);
    connect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), &exgTwo, SLOT(newBayerGRImage(cv::Mat, qint64)), Qt::QueuedConnection);

    // Initialize logger modules
    // Henrik: If the following line is not active a linker error is thrown ...
    LoggerModule test("../Logging", "Tester");
    this->loggerOne = new ImageLogger("../Logging", "CameraOne");
    this->loggerTwo = new ImageLogger("../Logging", "CameraTwo");
    
    // Connect cameras with the logging system
    connect(this->cameraOne, SIGNAL(newBayerGRImage(cv::Mat, qint64)), loggerOne, SLOT(burstImageLogger(cv::Mat, qint64)), Qt::QueuedConnection);
    connect(this->cameraTwo, SIGNAL(newBayerGRImage(cv::Mat, qint64)), loggerTwo, SLOT(burstImageLogger(cv::Mat, qint64)), Qt::QueuedConnection);

    drawGui();
    connect(&exgOne, SIGNAL(newImage(cv::Mat, qint64)), viewOne, SLOT(showImage(cv::Mat, qint64)));
    connect(&exgTwo, SIGNAL(newImage(cv::Mat, qint64)), viewTwo, SLOT(showImage(cv::Mat, qint64)));
    connect(cameraSettingsBtn, SIGNAL(pressed()), cameraOne, SLOT(showCameraSettings()));
}

void ATVImageLogger::drawGui(void )
{
    this->globalWidget = new QWidget(this);
    this->Layout = new QGridLayout(this->globalWidget);
    this->viewOne = new CQtOpenCVViewerGl(this);
    this->viewTwo = new CQtOpenCVViewerGl(this);
    this->LogBurstBtn = new QPushButton("Log image burst");
    connect(this->LogBurstBtn, SIGNAL(pressed()), loggerOne, SLOT(saveImageBurst()));
    connect(this->LogBurstBtn, SIGNAL(pressed()), loggerTwo, SLOT(saveImageBurst()));
    this->cameraSettingsBtn = new QPushButton("Camera settings");
    this->imageSelect = new QComboBox(globalWidget);
    //connect(this->imageSelect, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentViewChanged(QString)));
    connect(this->imageSelect, SIGNAL(currentIndexChanged(QString)), this, SLOT(imshowSelectorChanged(QString)));
    this->imageSelect->addItem("Input");
    this->imageSelect->addItem("Excess Green");
    this->imageSelect->addItem("Color");
    this->modicoviText = new QLabel("Modicovi Score:");
    this->sideWidget = new QWidget(globalWidget);
    this->sideLayout = new QGridLayout(this->sideWidget);
    this->viewOne->setMinimumHeight(400);
    this->viewOne->setMinimumWidth(1024);
    this->viewTwo->setMinimumHeight(400);
    this->viewTwo->setMinimumWidth(1024);
    this->Layout->addWidget(viewOne, 1,1);
    this->Layout->addWidget(viewTwo, 2,1);
    this->Layout->addWidget(imageSelect, 3,1);
    this->Layout->addWidget(sideWidget, 1,2);
    this->sideLayout->addWidget(LogBurstBtn, 2,1);
    this->sideLayout->addWidget(cameraSettingsBtn, 3,1);
    this->sideLayout->addWidget(modicoviText, 1,1);
    this->sideWidget->setLayout(this->sideLayout);
    this->globalWidget->setLayout(this->Layout);
    setCentralWidget(this->globalWidget);
}

void ATVImageLogger::currentViewChanged(const QString& text)
{
    std::cout << "Received new view " << text.toLocal8Bit().data() << std::endl;
    disconnect(this->viewOne, SLOT(showImage(cv::Mat*)));
}

void ATVImageLogger::imshowSelectorChanged(QString text)
{
    std::cout << "<imshowSelectorChanged(" << text.toLocal8Bit().data() << ")>" << std::endl;
}

ATVImageLogger::~ATVImageLogger()
{}

#include "ATVImageLogger.moc"
