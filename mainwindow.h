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

QT_CHARTS_USE_NAMESPACE


#include <apl/libapl.h>

// for settings
#define LOAD_WS    "load_ws"
#define CHART_TITLE "chart_title"
#define Y_TITLE    "y_title"
#define X_TITLE    "x_title"
#define X_VAR_NAME "x_var_name"
#define X_VAR_MIN  "x_var_min"
#define X_VAR_MAX  "x_var_max"
#define Z_TITLE    "z_title"
#define Z_VAR_NAME "z_var_name"
#define Z_VAR_MIN  "z_var_min"
#define Z_VAR_MAX  "z_var_max"
#define FUNCTION   "function"
#define FCN_LABEL  "function_label"
#define DO_SPLINE  "do_spline"
#define DO_POLAR   "do_polar"
#define THEME      "theme"
#define WIDTH      "width"
#define HEIGHT     "height"

typedef struct {
  QString tag;
  void (*handler)(QXmlStreamReader &stream);
  int idx;
  int logical;
} xml_tag_s;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void buildMenu (MainWindow *win, QChart *chart, QPolarChart *polarchart);
  static void handle_qvis  (QXmlStreamReader &stream);
  static void handle_curve (QXmlStreamReader &stream);
  static void handle_shorttitle (QXmlStreamReader &stream);
  static void handle_title (QXmlStreamReader &stream);
									  
public slots:

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void handleExpression ();
  void handleSettings ();
  void byebye ();
  void valChanged(bool enabled);
  void valChangedv();
  void themeChanged(int newtheme);
  void newFile();
  void open();
  bool save();
  bool saveAs();
  void about();
  
private:
  void           createActions();
  bool 		 saveFile (QString &fileName);
  void 		 readFile (QString &fileName);
  QChartView	*chartView;
  QChart	*chart;
  QPolarChart	*polarchart;
  QLineEdit	*chart_title;
  
  QLineEdit	*y_title;
  QLineEdit 	*apl_expression;
  QLineEdit 	*fcn_label;

  QLineEdit	*x_title;
  QLineEdit 	*x_var_name;
  QDoubleSpinBox *x_var_min;
  QDoubleSpinBox *x_var_max;

  QLineEdit 	*z_title;
  QLineEdit 	*z_var_name;
  QDoubleSpinBox *z_var_min;
  QDoubleSpinBox *z_var_max;

  QCheckBox 	*do_spline;
  QCheckBox 	*do_polar;
  QChart::ChartTheme theme;

  bool		 changed;
  QSettings 	 settings;
  QComboBox 	*themebox;
  QString 	 curFile;
  void		 create_menuBar ();
  int	         handle_vector (APL_value res,
				APL_value xvals,
				QString flbl);
  bool		 maybeSave();
  void		 initXmlHash ();
};
#endif // MAINWINDOW_H
