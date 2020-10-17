#include "mainwindow.h"
#include <QApplication>
#include <QSettings>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QApplication::setOrganizationName("Lumlum Software");
    QApplication::setApplicationName("PedalVibration");

    MainWindow w;
    (void)QObject::connect(&w, &MainWindow::quit, &a, &QApplication::quit);

    // Start minimized to avoid focus issues with the active Assetto Corsa window
    w.showMinimized();

    // Also call hide() to remove the taskbar icon
    w.hide();
    return a.exec();
}
