#include <QApplication>
#include <QDebug>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Hello World";

    MainWindow mw;
    mw.show();

    return a.exec();
}
