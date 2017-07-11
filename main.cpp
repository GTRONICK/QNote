#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if(argc >= 2){
        QStringList lobFilenames = QApplication::arguments();
        lobFilenames.removeFirst();
        w.setFileNameFromCommandLine(lobFilenames);
    }

    return a.exec();
}
