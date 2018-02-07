#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>

int main(int argc, char *argv[])
{
#ifdef __arm__
    qputenv("QMLSCENE_DEVICE", "epaper");
    qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180");
    qputenv("QT_QPA_GENERIC_PLUGINS", "evdevtablet");
#endif

    QApplication a(argc, argv);
    MainWindow w;

    qDebug() << a.primaryScreen()->geometry();
    w.show();

    //w.show();

    return a.exec();
}
