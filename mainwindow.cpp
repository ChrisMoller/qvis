#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <values.h>
#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"

#define expvar "expvarλ"

void
MainWindow::handleExpression ()
{
  QString xlbl = x_var_name->text ();
  QString xmin = x_var_min->text ();
  QString xmax = x_var_max->text ();

  QString zlbl = z_var_name->text ();
  QString zmin = z_var_min->text ();
  QString zmax = z_var_max->text ();

  QString input = apl_expression->text ();
  QString flbl  = fcn_label->text ();
  int incr = 16;

  if (!xlbl.isEmpty () && !xmin.isEmpty () && !xmax.isEmpty () &&
      !input.isEmpty ()) {
    settings.setValue (X_VAR_NAME, xlbl);
    settings.setValue (X_VAR_MIN,  xmin);
    settings.setValue (X_VAR_MAX,  xmax);
    settings.setValue (FUNCTION,   input);
    settings.setValue (FCN_LABEL,  flbl);
    /***
	lbl ← min + ((⍳incr+1)-⎕io) × (max - min) ÷ incr
    ***/
    QString range_x =
      QString ("%1 ← %2 + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
      .arg(xlbl).arg(xmin).arg(incr).arg(xmax);
    apl_exec (range_x.toStdString ().c_str ());
    APL_value xvals =
      get_var_value (xlbl.toStdString ().c_str (), "something");

    //    https://doc.qt.io/qt-5/qtcharts-splinechart-example.html

    bool zset = false;
    if (!zlbl.isEmpty () && !zmin.isEmpty () && !zmax.isEmpty ()) {
      settings.setValue (Z_VAR_NAME, zlbl);
      settings.setValue (Z_VAR_MIN,  zmin);
      settings.setValue (Z_VAR_MAX,  zmax);
      QString range_z =
	QString ("%1 ← %2 + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
	.arg(zlbl).arg(zmin).arg(incr).arg(zmax);
      zset = true;
      apl_exec (range_z.toStdString ().c_str ());
    }
  
    QString fcn = QString ("%1  ← %2").arg (expvar).arg (input);
    apl_exec (fcn.toStdString ().c_str ());

    APL_value res = get_var_value (expvar, "something");
    if (res) {
      lcl_chart->removeAllSeries();
      uint64_t count = get_element_count (res);
#ifdef POINTS
      QLineSeries *series = new QLineSeries ();
#else	// splines
      QSplineSeries *series = new QSplineSeries ();
#endif
      qreal y_max = -MAXDOUBLE;
      qreal y_min =  MAXDOUBLE;
      for (uint64_t i = 0; i < count; i++) {
	qreal y_val = (qreal)get_real (res, i);
	if (y_max < y_val) y_max = y_val;
	if (y_min > y_val) y_min = y_val;
	series->append ((qreal)get_real (xvals, i), y_val);
      }
      series->setName(flbl);
      lcl_chart->addSeries (series);
      lcl_chart->createDefaultAxes ();
      /***
	  QChart::ChartThemeLight
	  QChart::ChartThemeBlueCerulean
	  QChart::ChartThemeDark
	  QChart::ChartThemeBrownSand
	  QChart::ChartThemeBlueNcs
	  QChart::ChartThemeHighContrast
	  QChart::ChartThemeBlueIcy
	  QChart::ChartThemeQt
       ***/
      lcl_chart->setTheme (QChart::ChartThemeBlueCerulean);
    
      qreal dy = 0.075 * (y_max - y_min);
      lcl_chart->axes (Qt::Vertical).first()->setRange(y_min-dy, y_max+dy);
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
MainWindow::buildMenu (MainWindow *win)
{
  QGroupBox *formGroupBox = new QGroupBox ("Visualisation");
  QGridLayout *layout = new QGridLayout;

  QSettings settings;

#if 0
  /*   compute button   */

  QString compute_button_style ("background-color: yellow; color: red;");
  QFont   compute_button_font ("bold");
  QPushButton *compute_button = new QPushButton (QObject::tr ("Switch curves"));
  compute_button->setStyleSheet (compute_button_style);
  compute_button->setFont (compute_button_font);
  compute_button->setToolTip ("Switch curves");
  layout->addWidget (compute_button, 0, 0);

  QObject::connect (compute_button,
		    SIGNAL (clicked ()),
		    win,
		    SLOT (setValue()));
#endif

  /*  x indep vbl */

  QString xlbl = settings.value (X_VAR_NAME).toString ();
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  x_var_name->setText (xlbl);
  layout->addWidget (x_var_name, 0, 0);
  
  QString xmin = settings.value (X_VAR_MIN).toString ();
  x_var_min = new  QLineEdit ();
  x_var_min->setPlaceholderText ("x minimum value");
  x_var_min->setText (xmin);
  layout->addWidget (x_var_min, 0, 1);

  QString xmax = settings.value (X_VAR_MAX).toString ();
  x_var_max = new  QLineEdit ();
  x_var_max->setPlaceholderText ("x maximum value");
  x_var_max->setText (xmax);
  layout->addWidget (x_var_max, 0, 2);

  /*  z indep vbl */

  QString zlbl = settings.value (Z_VAR_NAME).toString ();
  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  z_var_name->setText (zlbl);
  layout->addWidget (z_var_name, 1, 0);

  QString zmin = settings.value (Z_VAR_MIN).toString ();
  z_var_min = new  QLineEdit ();
  z_var_min->setPlaceholderText ("z minimum value");
  z_var_min->setText (zmin);
  layout->addWidget (z_var_min, 1, 1);

  QString zmax = settings.value (Z_VAR_MAX).toString ();
  z_var_max = new  QLineEdit ();
  z_var_max->setPlaceholderText ("z maximum value");
  z_var_max->setText (zmax);
  layout->addWidget (z_var_max, 1, 2);


  /*  APL expression */

  QString flbl = settings.value (FCN_LABEL).toString ();
  fcn_label = new  QLineEdit ();
  fcn_label->setPlaceholderText ("curve label");
  fcn_label->setText (flbl);
  layout->addWidget (fcn_label, 2, 0);

  QString fcn = settings.value (FUNCTION).toString ();
  apl_expression = new  QLineEdit ();
  apl_expression->setText (fcn);
  layout->addWidget (apl_expression, 2, 1, 1, 2);

  QObject::connect (apl_expression,
		    &QLineEdit::editingFinished,
		    this,
		    &MainWindow::handleExpression);



  /*   quit button   */

  QString quit_button_style ("background-color: red; color: yellow;");
  QFont   quit_button_font ("bold");
  QPushButton *quit_button = new QPushButton (QObject::tr ("Quit"));
  quit_button->setStyleSheet (quit_button_style);
  quit_button->setFont (quit_button_font);
  quit_button->setToolTip ("Quit");
  layout->addWidget (quit_button, 3, 2);

  QObject::connect (quit_button,
		    &QPushButton::clicked,
		    win,
		    &QCoreApplication::quit);



  /*   end buttons  */

  formGroupBox->setLayout (layout);
  formGroupBox->show ();
}

MainWindow::MainWindow (QChartView *chartView, QWidget *parent)
  : QMainWindow(parent)
{
  lcl_chart = chartView->chart ();
  chartView->setRenderHint(QPainter::Antialiasing);

  buildMenu (this);

  handleExpression ();
}

MainWindow::~MainWindow()
{

}

