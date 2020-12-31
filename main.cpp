#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QXYSeries>
#include <QtCharts/QLineSeries>
#include <QtMath>

#include "mainwindow.h"

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

int
main (int argc, char *argv[])
{
  init_libapl ("apl", 0);

  QApplication app (argc, argv);
  QCoreApplication::setOrganizationName("MSDS");
  QCoreApplication::setApplicationName("APL Visualiser");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("APL Visualiser");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption noload ("N", "Skip loading saved workspace.");
  parser.addOption(noload);
  
  QCommandLineOption
    loadws(QStringList() << "L" << "workspace", "workspace", "<ws>");
  parser.addOption(loadws);

  parser.process(app);

  QSettings settings;

  QString ws = parser.value(loadws);
  if (ws.isEmpty () && !parser.isSet (noload)) 
    ws = settings.value (LOAD_WS).toString ();

  if (!ws.isEmpty ()) {
    std::string cmd = ")load " + ws.toStdString ();
    apl_command (cmd.c_str ());
    settings.setValue (LOAD_WS, ws);
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
