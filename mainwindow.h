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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QPolarChart>
#include <QTextEdit>
#include <QObject>

QT_CHARTS_USE_NAMESPACE

#include <apl/libapl.h>

#include "history.h"
#include "curves.h"
#include "chartdata.h"

#include "QtColorWidgets/color_selector.hpp"
using namespace color_widgets;

// for settings
#define SETTINGS_EDITOR     "Editor"
#define SETTINGS_FONT       "Font"
#define SETTINGS_STYLE      "Style"
#define SETTINGS_HEIGHT     "Height"
#define SETTINGS_WIDTH      "Width"

#define toCString(v)	((v).toStdString ().c_str ())

typedef enum {
  SAVE_MODE_NONE,
  SAVE_MODE_SAVE,
  SAVE_MODE_DUMP,
  SAVE_MODE_OUT
} save_mode_e;

#if 1
class OldCurve
{
public:
  bool polar;
  bool spline;
  QString shorttitle;
  QString title;
  //Function function;
  //Index ix;
  //Index iz;
};
#endif

typedef struct {
  QString tag;
  int idx;
  bool logical;
} xml_tag_s;

class MainWindow;

class AplLineFilter : public QObject
{
    Q_OBJECT
  
public:
  AplLineFilter (QLineEdit *obj, MainWindow *mw)
  {watched = obj; mainwin = mw;}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  MainWindow *mainwin;
  QLineEdit *watched;
};

class AplWinFilter : public QObject
{
    Q_OBJECT
  
public:
  AplWinFilter (QTextEdit *obj, MainWindow *mw)
  {watched = obj; mainwin = mw;}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  MainWindow *mainwin;
  QTextEdit *watched;
};

class ChartWindow;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow (QString &msgs, QStringList &args,
	      QString &lp, QWidget *parent = nullptr);
  ~MainWindow ();
  History 	*history;
  QLineEdit	*aplline;
  QTextEdit 	*aplwin;
  int 		 getCurveCount () { return curves.size (); }
  Curve		 getCurve (int i) { return curves[i]; }
  int 		 getChartCount () { return charts.size (); }
  ChartData	*getChart (int i) {
    return ((i >= 0 && i < charts.size ()) ? charts[i] : nullptr);
  }
  void		 initXmlHash ();
  void 		 setTabTitle (int ix, QString &title)
      { tabs->setTabText (ix, title); }
									  
public slots:

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void handleExpression ();
  void byebye ();
  void loadapl();
  bool save();
  bool saveAs();
  bool saveVis();
  bool saveAsVis();
  void addCurve();
  void newChart();
  void about();
  void returnPressed ();
  void fileChanged(const QString &path);
  void cellPressed (int row, int column);
  bool writeVis (QString &fileName);
  void readVis (QString &fileName);
  void openVis ();
  
  
private:
  void 		 setEditor ();
  void 		 setGlobalStyle ();
  void		 setGlobalFont ();
  void		 openapl(bool cpy);
  void           createActions();
  void		 process_line(QString text);
  void		 loadLastSession ();
  void		 create_menuBar ();
  bool		 maybeSave();
  void 		 buildMenu (QString &msgs);
  QGroupBox *	 chartControls ();
  QComboBox 	 *themebox;
  bool		  changed;
  QString 	  curFile;
  QChart::ChartTheme theme;	// fixme--copy to chartwin
  ChartWindow	 *chartWindow;
  void		  insertItem (int i, QTableWidget* &curvesTable);
  Index		 *parseIdx (QXmlStreamReader &stream);
  Index 	 *parseIx (QXmlStreamReader &stream);
  Index		 *parseIz (QXmlStreamReader &stream);
  bool 		  parseCurves (QXmlStreamReader &stream);
  bool 		  parseCurve (int idx, QXmlStreamReader &stream);
  bool 		  parseCharts (QXmlStreamReader &stream);
  bool 		  parseChart (bool spline, bool polar, int theme,
			      QXmlStreamReader &stream);
  
private:
  AplLineFilter		*aplLineFilter;
  AplWinFilter		*aplWinFilter;
  QString		libpath;
  QFileSystemWatcher	watcher;
  void update_screen (QString &errString, QString &outString);
  QString		editor;
  QTemporaryDir		tempdir;
  save_mode_e		save_mode;
  QList<Curve>		curves;
  QTabWidget 		*tabs;
  QTableWidget 		*curvesTable;
  QList<ChartData*>	charts;
  QString		styledesc;
};
#endif // MAINWINDOW_H
