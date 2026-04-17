#include "customgraphicswidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CustomGraphicsWidget w;
    w.show();
    return QCoreApplication::exec();
}
