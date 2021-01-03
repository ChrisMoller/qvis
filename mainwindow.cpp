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
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QPolarChart>
#include <values.h>
#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"

#define expvar "expvarλ"

void
MainWindow::closeEvent(QCloseEvent *event __attribute__((unused)))
{
  fprintf (stderr, "byebye\n");
}

void
MainWindow::themeChanged (int newtheme __attribute__((unused)))
{
  theme = (QChart::ChartTheme)themebox->currentData ().toInt ();
  handleExpression ();
}


void
MainWindow::byebye ()
{
  settings.setValue (HEIGHT, lcl_chartView->height ());
  settings.setValue (WIDTH, lcl_chartView->width ());
  QCoreApplication::quit ();
}

void
MainWindow::handleSettings ()
{
  QGridLayout *layout = new QGridLayout;
  
  themebox = new QComboBox ();
  themebox->addItem ("Light", QChart::ChartThemeLight);
  themebox->addItem ("Blue Cerulean", QChart::ChartThemeBlueCerulean);
  themebox->addItem ("Dark", QChart::ChartThemeDark);
  themebox->addItem ("Brown Sand", QChart::ChartThemeBrownSand);
  themebox->addItem ("Blue Ncs", QChart::ChartThemeBlueNcs);
  themebox->addItem ("High Contrast", QChart::ChartThemeHighContrast);
  themebox->addItem ("Blue Icy", QChart::ChartThemeBlueIcy);
  themebox->addItem ("Qt", QChart::ChartThemeQt);

  themebox->setCurrentIndex (3);
  
  connect(themebox,
	  QOverload<int>::of(&QComboBox::activated),
	  this,
	  &MainWindow::themeChanged);
  
  layout->addWidget(themebox, 0, 0);
  
  QDialog dialog;
  dialog.setLayout (layout);
  dialog.exec ();
}

void
MainWindow::handleExpression ()
{
  Qt::CheckState polar_checked = do_polar->checkState();
  bool is_polar = (polar_checked == Qt::Checked) ? true : false;
  settings.setValue (DO_POLAR, is_polar);
  lcl_chartView->setChart (is_polar ? lcl_polarchart : lcl_chart);
		     
  QString xlbl = x_var_name->text ();
  double  xmin = x_var_min->value ();
  double  xmax = x_var_max->value ();

  QString zlbl = z_var_name->text ();
  double  zmin = z_var_min->value ();
  double  zmax = z_var_max->value ();

  QString input = apl_expression->text ();
  QString flbl  = fcn_label->text ();
  int incr = 16;

  if (!xlbl.isEmpty () && !input.isEmpty ()) {
    settings.setValue (X_VAR_NAME, xlbl);
    settings.setValue (X_VAR_MIN,  xmin);
    settings.setValue (X_VAR_MAX,  xmax);
    settings.setValue (FUNCTION,   input);
    settings.setValue (FCN_LABEL,  flbl);
    /***
	lbl ← min + ((⍳incr+1)-⎕io) × (max - min) ÷ incr
    ***/
    QString range_x =
      QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
      .arg(xlbl).arg(xmin).arg(incr).arg(xmax);
    apl_exec (range_x.toStdString ().c_str ());
    APL_value xvals =
      get_var_value (xlbl.toStdString ().c_str (), "something");

    //    https://doc.qt.io/qt-5/qtcharts-splinechart-example.html

    bool zset = false;
    if (!zlbl.isEmpty ()) {
      settings.setValue (Z_VAR_NAME, zlbl);
      settings.setValue (Z_VAR_MIN,  zmin);
      settings.setValue (Z_VAR_MAX,  zmax);
      QString range_z =
	QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
	.arg(zlbl).arg(zmin).arg(incr).arg(zmax);
      zset = true;
      apl_exec (range_z.toStdString ().c_str ());
    }
  
    QString fcn = QString ("%1  ← %2").arg (expvar).arg (input);
    apl_exec (fcn.toStdString ().c_str ());

    APL_value res = get_var_value (expvar, "something");
    if (res) {
      lcl_chartView->chart ()->removeAllSeries();
      uint64_t count = get_element_count (res);
      qreal y_max = -MAXDOUBLE;
      qreal y_min =  MAXDOUBLE;

      QSplineSeries *sseries = nullptr;
      QLineSeries   *pseries = nullptr;
      Qt::CheckState spline_checked = do_spline->checkState();
      settings.setValue (DO_SPLINE,  spline_checked);
      if (spline_checked == Qt::Checked) 
	sseries = new QSplineSeries ();
      else 
	pseries = new QLineSeries ();
      for (uint64_t i = 0; i < count; i++) {
	qreal y_val = (qreal)get_real (res, i);
	if (y_max < y_val) y_max = y_val;
	if (y_min > y_val) y_min = y_val;
	if (sseries) sseries->append ((qreal)get_real (xvals, i), y_val);
	else pseries->append ((qreal)get_real (xvals, i), y_val);
      }
      if (sseries) {
	sseries->setName(flbl);
	lcl_chartView->chart ()->addSeries (sseries);
      }
      else {
	pseries->setName(flbl);
	lcl_chartView->chart ()->addSeries (pseries);
      }

      settings.setValue (HEIGHT, lcl_chartView->height ());
      settings.setValue (WIDTH, lcl_chartView->width ());
      lcl_chartView->chart ()->createDefaultAxes ();

      settings.setValue (THEME,  theme);
      lcl_chartView->chart ()->setTheme (theme);

      QString ttl = chart_title->text ();
      settings.setValue (CHART_TITLE,  ttl);
      lcl_chartView->chart ()->setTitle (ttl);
      
      QString x_ttl = x_title->text ();
      settings.setValue (X_TITLE,  x_ttl);
      lcl_chartView->chart ()->axes (Qt::Horizontal).first()
	->setTitleText(x_ttl);
      
      QString y_ttl = y_title->text ();
      settings.setValue (Y_TITLE,  y_ttl);
      lcl_chartView->chart ()->axes (Qt::Vertical).first()
	->setTitleText(y_ttl);
    
      qreal dy = 0.075 * (y_max - y_min);
      lcl_chartView->chart ()->axes (Qt::Vertical).first()
	->setRange(y_min-dy, y_max+dy);

      QString cmd = QString (")erase %1 %2").arg (expvar).arg (xlbl);
      apl_command (cmd.toStdString ().c_str ());
      if (zset) {
	cmd = QString (")erase %1").arg (zlbl);
	apl_command (cmd.toStdString ().c_str ());
      }
    }
  }
}

void
MainWindow::valChanged (bool enabled __attribute__((unused)))
{
  handleExpression ();
}

void
MainWindow::buildMenu (MainWindow *win, QChart *chart,
			QPolarChart *polarchart)
{
  QGroupBox *formGroupBox = new QGroupBox ("Visualisation");
  QGridLayout *layout = new QGridLayout;

  QSettings settings;

  int row = 0;
  int col = 0;

  QString ttl = settings.value (CHART_TITLE).toString ();
  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  chart_title->setText (ttl);
  layout->addWidget (chart_title, row, 0, 1, 3);
  
  QString y_text = settings.value (Y_TITLE).toString ();
  y_title = new  QLineEdit ();
  y_title->setPlaceholderText ("y axix label");
  y_title->setText (y_text);
  layout->addWidget (y_title, row, 3);
  
  /*  x indep vbl */

  row++;
  col = 0;
  
  QString xlbl = settings.value (X_VAR_NAME).toString ();
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  x_var_name->setText (xlbl);
  layout->addWidget (x_var_name, row, col++);
  
  double xmin = settings.value (X_VAR_MIN).toDouble ();
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_min->setToolTip ("x minimum value");
  x_var_min->setValue (xmin);
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_min, row, col++);

  double xmax = settings.value (X_VAR_MAX).toDouble ();
  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_max->setToolTip ("x maximum value");
  x_var_max->setValue (xmax);
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_max, row, col++);
  
  QString x_text = settings.value (X_TITLE).toString ();
  x_title = new  QLineEdit ();
  x_title->setPlaceholderText ("x axix label");
  x_title->setText (x_text);
  layout->addWidget (x_title, row, col++);

  /*  z indep vbl */

  row++;
  col = 0;

  QString zlbl = settings.value (Z_VAR_NAME).toString ();
  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  z_var_name->setText (zlbl);
  layout->addWidget (z_var_name, row, col++);

  double zmin = settings.value (Z_VAR_MIN).toDouble ();
  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_min->setToolTip ("z minimum value");
  z_var_min->setValue (zmin);
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_min, row, col++);

  double zmax = settings.value (Z_VAR_MAX).toDouble ();
  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_max->setToolTip ("z maximum value");
  z_var_max->setValue (zmax);
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_max, row, col++);
  
  QString z_text = settings.value (Z_TITLE).toString ();
  z_title = new  QLineEdit ();
  z_title->setPlaceholderText ("z axix label");
  z_title->setText (z_text);
  layout->addWidget (z_title, row, col++);


  /*  APL expression */

  row++;
  col = 0;
  QString flbl = settings.value (FCN_LABEL).toString ();
  fcn_label = new  QLineEdit ();
  fcn_label->setPlaceholderText ("curve label");
  fcn_label->setText (flbl);
  layout->addWidget (fcn_label, row, col++);

  QString fcn = settings.value (FUNCTION).toString ();
  apl_expression = new  QLineEdit ();
  apl_expression->setPlaceholderText ("function");
  apl_expression->setText (fcn);
  layout->addWidget (apl_expression, row, col, 1, 3);
  QObject::connect (apl_expression,
		    &QLineEdit::editingFinished,
		    this,
		    &MainWindow::handleExpression);


  /*  toggles */
  
  row++;
  col = 0;
  
  bool dospl = settings.value (DO_SPLINE).toBool ();
  do_spline = new QCheckBox ("Spline");
  do_spline->setCheckState (dospl ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);
  
  bool dopol = settings.value (DO_POLAR).toBool ();
  do_polar = new QCheckBox ("Polar");
  do_polar->setCheckState (dopol ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);


  /*   buttons */

  /*   settings button   */

  row++;
  QString settings_button_style ("background-color: yellow; color: green;");
  QFont   settings_button_font ("bold");
  QPushButton *settings_button = new QPushButton (QObject::tr ("Settings"));
  settings_button->setStyleSheet (settings_button_style);
  settings_button->setFont (settings_button_font);
  settings_button->setToolTip ("Set parameters");
  layout->addWidget (settings_button, row, 0);
  QObject::connect (settings_button,
		    SIGNAL (clicked ()),
		    win,
		    SLOT (handleSettings()));

  /*   compute button   */

  QString compute_button_style ("background-color: yellow; color: red;");
  QFont   compute_button_font ("bold");
  QPushButton *compute_button = new QPushButton (QObject::tr ("Draw"));
  compute_button->setStyleSheet (compute_button_style);
  compute_button->setFont (compute_button_font);
  compute_button->setToolTip ("Draw");
  layout->addWidget (compute_button, row, 1);
  QObject::connect (compute_button,
		    SIGNAL (clicked ()),
		    win,
		    SLOT (handleExpression()));

  /*   quit button   */

  QString quit_button_style ("background-color: red; color: yellow;");
  QFont   quit_button_font ("bold");
  QPushButton *quit_button = new QPushButton (QObject::tr ("Quit"));
  quit_button->setStyleSheet (quit_button_style);
  quit_button->setFont (quit_button_font);
  quit_button->setToolTip ("Quit");
  layout->addWidget (quit_button, row, 3);

  QObject::connect (quit_button,
		    SIGNAL (clicked ()),
		    win,
		    SLOT (byebye()));



  /*   end buttons  */

  formGroupBox->setLayout (layout);
  formGroupBox->show ();

  lcl_chartView->setChart (dopol ? polarchart : chart);
}

MainWindow::MainWindow (QChartView *chartView, QChart *chart,
			QPolarChart *polarchart, QWidget *parent)
  : QMainWindow(parent)
{

  QVariant ww = settings.value (WIDTH);
  QVariant hh = settings.value (HEIGHT);
  if (ww.isValid () && hh.isValid ()) 
    this->resize (ww.toInt (), hh.toInt ());
  
  lcl_chartView  = chartView;
  lcl_chart      = chart;
  lcl_polarchart = polarchart;
  chartView->setRenderHint(QPainter::Antialiasing);

  QVariant tt = settings.value (THEME);
  theme = tt.isValid ()
    ? (QChart::ChartTheme)tt.toInt ()
    :  QChart::ChartThemeBlueCerulean;

  buildMenu (this, chart, polarchart);

  handleExpression ();
}

MainWindow::~MainWindow()
{

}

