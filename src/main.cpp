#include <QtGui/QApplication>
#include "VisionSpray.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MoDiCoVi foo;
    foo.show();
    return app.exec();
}
