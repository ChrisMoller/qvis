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

#if 0
  apl_exec("a←1 2 3 + 4 5 6");
  APL_value a = get_var_value ("a", "something");
  qInfo ("rank  = %d\n", get_rank (a));
  uint64_t count = get_element_count (a);
  qInfo ("a = [%d] ", (int)count);
  for (uint64_t i = 0; i < count; i++)
    qInfo (" %d ", (int)get_int (a, i));
  qInfo ("\n");
#endif

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
