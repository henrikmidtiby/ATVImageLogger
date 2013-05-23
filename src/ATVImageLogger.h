#ifndef ATVImageLogger_H
#define ATVImageLogger_H


#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QComboBox>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QDir>    

#include "../include/qOpenGLCVWidget/qOpenGLCVWidget.h"
#include "demosaic_cv.h"
#include "../include/BayerExG/exg_cv.h"
#include "../include/QtGigE/qtgige.h"

#ifdef USE_DATALOGGER
  #include "../include/LoggerModule/datalogger.h"
#endif
#include "../include/LoggerModule/loggermodule.h"
#include "../include/LoggerModule/imagelogger.h"

class ATVImageLogger : public QMainWindow
{
Q_OBJECT
public:
    ATVImageLogger();
    virtual ~ATVImageLogger();
    QTGIGE * cameraOne;
    QTGIGE * cameraTwo;
private:
    void startCameras();
    QTGIGE * startCamera(char* deviceId);
    void startLoggingSystem();
    void connectImageProducersToGui();
    void connectGuiElements();
    QPushButton * LogBurstBtn;
    QPushButton * LogSingleImageBtn;
    QPushButton * cameraSettingsOneBtn;
    QPushButton * cameraSettingsTwoBtn;
    CQtOpenCVViewerGl * viewOne;
    CQtOpenCVViewerGl * viewTwo;
    void drawGui(void);
    QGridLayout *Layout;
    QGridLayout *sideLayout;
    QWidget *globalWidget;
    QWidget *sideWidget;
    QComboBox *imageSelect;
    QLabel * modicoviText;
    demosaic_cv demOne;
    demosaic_cv demTwo;
    ExG_cv exgOne;
    ExG_cv exgTwo;
    ImageLogger * loggerOne;
    ImageLogger * loggerTwo;
    
private slots:
    void currentViewChanged ( const QString & text );
    void imshowSelectorChanged(QString text);
};

#endif // ATVImageLogger_H
