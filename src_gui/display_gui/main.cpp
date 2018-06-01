#include <QGuiApplication>
#include <QDir>
#include <cassert>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCore/QDateTime>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCharts/QValueAxis>
#include "simplexy.h"

int main(int argc, char *argv[])
{
    using namespace QtCharts;
    QApplication a(argc, argv);
    {
        QDir dir(QApplication::applicationDirPath());
        QStringList lib_paths = QApplication::libraryPaths();
        lib_paths << QApplication::applicationDirPath();
#ifdef __MACH__
        lib_paths << dir.absolutePath() << dir.absolutePath() + "/plugins";
        dir.cdUp();
        lib_paths << dir.absolutePath();
        dir.cd("PlugIns");
        lib_paths << dir.absolutePath();
#endif
        dir.cd("plugins");
        lib_paths << dir.absolutePath();
        dir.cd("platforms");
        lib_paths << dir.absolutePath();
        dir.cd("imageformats");
        lib_paths << dir.absolutePath();
        QApplication::setLibraryPaths(lib_paths);
    }
    QMainWindow window;
    {
        SimpleXY *series = new SimpleXY();

        QChart *chart = new QChart();
        chart->legend()->hide();
        chart->addSeries(series);
        chart->createDefaultAxes();
        chart->setTitle("Simple line chart example");

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        window.setCentralWidget(chartView);
        window.resize(400, 300);
        window.show();
    }

    return a.exec();
}
