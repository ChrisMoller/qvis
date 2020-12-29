#include "mainwindow.h"

#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QXYSeries>
#include <QtCharts/QLineSeries>
#include <QtMath>

QT_CHARTS_USE_NAMESPACE


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLineSeries *series = new QLineSeries();

    qreal x;
    for (x = 0.0; x < 6.28; x += 0.1) series->append(x, qSin(x));

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    MainWindow window(chartView);
    window.setCentralWidget(chartView);
    window.resize(400, 300);
    window.show();
    return app.exec();
}
