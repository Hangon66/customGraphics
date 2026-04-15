#include "graphicstestwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GraphicsTestWidget w;
    w.show();
    return QCoreApplication::exec();
}
