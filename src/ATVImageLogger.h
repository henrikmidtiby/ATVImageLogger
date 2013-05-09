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

#ifdef USE_CAMERA
  #pragma message "Using a camera"
  #include "../include/QtGigE/qtgige.h"
#endif

#ifdef USE_DATALOGGER
  #include "../include/LoggerModule/datalogger.h"
#endif
#include "../include/LoggerModule/loggermodule.h"

class ATVImageLogger : public QMainWindow
{
Q_OBJECT
public:
    ATVImageLogger();
    virtual ~ATVImageLogger();
#ifdef USE_CAMERA
    QTGIGE * camera;
#endif
private:
    QPushButton * Valve1Btn;
    QPushButton * Valve2Btn;
    QPushButton * cameraSettingsBtn;
    CQtOpenCVViewerGl * view;
    void drawGui(void);
    QGridLayout *Layout;
    QGridLayout *sideLayout;
    QWidget *globalWidget;
    QWidget *sideWidget;
    QComboBox *imageSelect;
    QLabel * modicoviText;
    demosaic_cv dem;
    ExG_cv exg;
    
private slots:
    void currentViewChanged ( const QString & text );
};

#endif // ATVImageLogger_H
