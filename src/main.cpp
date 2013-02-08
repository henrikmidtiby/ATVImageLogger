#include <QtGui/QApplication>
#include "VisionSpray.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_X11InitThreads);
    VisionSpray foo;
    foo.show();
    return app.exec();
}
