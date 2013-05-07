#include <QtGui/QApplication>
#include "ATVImageLogger.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_X11InitThreads);
    ATVImageLogger foo;
    foo.show();
    return app.exec();
}
