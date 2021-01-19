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

int
main (int argc, char *argv[])
{
  // supress anoying messages
  qputenv("QT_LOGGING_RULES","*.debug=false;qt.qpa.*=false");

  init_libapl ("apl", 0);

  QApplication app (argc, argv);
  QCoreApplication::setOrganizationName("MSDS");
  QCoreApplication::setApplicationName("APL Visualiser");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("APL Visualiser");
  parser.addHelpOption();
  parser.addVersionOption();

#ifdef USE_SETTINGS
  QCommandLineOption norestore ("X", "Skip loading previous session.");
  parser.addOption(norestore);

  QCommandLineOption noload ("N", "Skip loading saved workspace.");
  parser.addOption(noload);
#endif
  
  QCommandLineOption
    loadws("L", "Workspace to load.", "<ws>");
  parser.addOption(loadws);

  parser.process(app);

#ifdef USE_SETTINGS
  bool do_restore = !parser.isSet (norestore);

  QSettings settings;

  QString ws = parser.value(loadws);
  if (ws.isEmpty () && !parser.isSet (noload)) 
    ws = settings.value (LOAD_WS).toString ();

  if (!ws.isEmpty ()) {
    std::string cmd = ")load " + ws.toStdString ();
    const char *rc = apl_command (cmd.c_str ());
    if (rc) free ((void *)rc);
    settings.setValue (LOAD_WS, ws);
  }

  MainWindow window (do_restore, nullptr);
#else
  QStringList vals = parser.values (loadws);
  QString startupMsgs;
  int i;
  for (i = 0; i < vals.count (); i++) {
    std::string op = (i == 0) ? ")load " : ")copy ";
    std::string cmd = op + vals.value (i).toStdString ();
    const char *rc = apl_command (cmd.c_str ());
    if (rc) {
      QString line = QString ("%1%2: %3")
	.arg (op.c_str ()).arg (vals.value (i)).arg (rc);
      startupMsgs.append (line);
      free ((void *)rc);
    }
  }

  QStringList args = parser.positionalArguments();
  MainWindow window (startupMsgs, args, nullptr);
#endif

  return app.exec ();
}
