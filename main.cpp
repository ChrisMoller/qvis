#include "mainwindow.h"

#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QXYSeries>
#include <QtCharts/QLineSeries>
#include <QtMath>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

int
main (int argc, char *argv[])
{
  init_libapl ("apl", 0);

  QApplication app (argc, argv);

  QChart *chart = new QChart ();

  QChartView *chartView = new QChartView (chart);
  chartView->setRenderHint (QPainter::Antialiasing);

  MainWindow window (chartView);
  window.setCentralWidget (chartView);
  window.resize (400, 300);
  window.show ();
  return app.exec ();
}
