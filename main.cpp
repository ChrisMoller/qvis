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
  QCoreApplication::setApplicationName("APL Visualiser");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("APL Visualiser");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption
    loadws(QStringList() << "L" << "workspace", "workspace", "<ws>");
  parser.addOption(loadws);

  parser.process(app);

  QString ws = parser.value(loadws);

  if (!ws.isEmpty ()) {
    std::string cmd = ")load " + ws.toStdString ();
    const char *rc = apl_command (cmd.c_str ());
  }
  
  QChart *chart = new QChart ();

  QChartView *chartView = new QChartView (chart);
  chartView->setRenderHint (QPainter::Antialiasing);

  MainWindow window (chartView);
  window.setCentralWidget (chartView);
  window.resize (400, 300);
  window.show ();
  return app.exec ();
}
