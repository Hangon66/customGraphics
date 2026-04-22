#include "customgraphicswidget.h"
#include "view/ShapeMetadata.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<PropField>();
    qRegisterMetaType<PropMap>();
    QApplication a(argc, argv);
    CustomGraphicsWidget w;
    w.show();
    return QCoreApplication::exec();
}
