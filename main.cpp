#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/resources/style/mainwindow.qss");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream ts(&file);
        QString styleSheet = ts.readAll();
        a.setStyleSheet(styleSheet);
        file.close();
    }
    else
    {
        qWarning("Could not open stylesheet file.");
    }
    QCoreApplication::setOrganizationName("Egor Belov");
    QCoreApplication::setApplicationName("VisualizerOfMathematicalFunctions");

    MainWindow window;
    window.show();
    return a.exec();
}
