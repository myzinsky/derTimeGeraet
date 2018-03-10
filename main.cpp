#include "dertimegeraet.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    derTimeGeraet w;
    w.show();

    return a.exec();
}
