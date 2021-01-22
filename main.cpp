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
#include "aplexec.h"

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

  QCommandLineOption noCONT ("noCONT",
			     "Skip loading SETUP or CONTINUE workspace.");
  parser.addOption(noCONT);
  
  QCommandLineOption
    loadws("L", "Workspace to load.", "<ws>");
  parser.addOption(loadws);

  parser.process(app);


  const char *rc;
  QString startupMsgs;
  bool cont_loaded = false;
  bool setup_loaded = false;
  if (!parser.isSet (noCONT)) {
    QString outString;
    QString errString;
    QString cmd = QString (")load CONTINUE");
    AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
    if (!outString.isEmpty ()) {
      cont_loaded = outString.contains ("SAVED");
      if (cont_loaded) {
	startupMsgs.append ("CONTINUE: ");
	startupMsgs.append (outString);
      }
      else {
	cmd = QString (")load SETUP");
	AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
	if (!outString.isEmpty ()) {
	  startupMsgs.append ("SETUP: ");
	  startupMsgs.append (outString);
	}
      }
    }
  }
  QStringList vals = parser.values (loadws);
  int i;
  bool something_loaded = cont_loaded || setup_loaded;
  for (i = 0; i < vals.count (); i++) {
    std::string op = something_loaded ? ")copy " : ")load ";
    std::string cmd = op + vals.value (i).toStdString ();
    rc = apl_command (cmd.c_str ());
    if (rc) {
      QString line = QString ("%1%2: %3")
	.arg (op.c_str ()).arg (vals.value (i)).arg (rc);
      something_loaded |= line.contains ("SAVED");
      startupMsgs.append (line);
      free ((void *)rc);
    }
  }

  QStringList args = parser.positionalArguments();
  MainWindow window (startupMsgs, args, nullptr);

  return app.exec ();
}
