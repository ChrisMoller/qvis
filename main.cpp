/***
    qvis edif Copyright (C) 2021  Dr. C. H. L. Moller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***/

#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QXYSeries>
#include <QtCharts/QLineSeries>
#include <QtMath>

#include <apl/libapl.h>


#include "mainwindow.h"

QT_CHARTS_USE_NAMESPACE

static QChartView *chartView = nullptr;

void
QGraphicsView::mousePressEvent(QMouseEvent *event)
{
  // https://doc.qt.io/qt-5/qgraphicsview.html#mousePressEvent
  // https://doc.qt.io/qt-5/qmouseevent.html
  if (event->button () == 2) {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
      fileNames = dialog.selectedFiles();
      QString fn = fileNames.first ();
      QPixmap p = chartView->grab();
      p.save(fn);
    }
  }
}

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
    const char *rc = apl_command (cmd.c_str ());
    fprintf (stderr, "rc = \"%s\"\n", rc);
    settings.setValue (LOAD_WS, ws);
  }
  
  QChart *chart      = new QChart ();
  QPolarChart *polarchart = new QPolarChart ();

  chartView = new QChartView ();
  MainWindow window (chartView, chart, polarchart);
  chartView->setRenderHint (QPainter::Antialiasing);

  window.setCentralWidget (chartView);
  window.resize (400, 300);
  window.show ();
  return app.exec ();
}
