#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
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
  int incr = 16;

  if (!xlbl.isEmpty () && !xmin.isEmpty () && !xmax.isEmpty () &&
      !input.isEmpty ()) {
    /***
	lbl ← min + ((⍳incr+1)-⎕io) × (max - min) ÷ incr
    ***/
    QString range_x =
      QString ("%1 ← %2 + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
      .arg(xlbl).arg(xmin).arg(incr).arg(xmax);
    apl_exec (range_x.toStdString ().c_str ());

    bool zset = false;
    if (!zlbl.isEmpty () && !zmin.isEmpty () && !zmax.isEmpty ()) {
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
      QLineSeries *series = new QLineSeries ();
      uint64_t count = get_element_count (res);
      for (uint64_t i = 0; i < count; i++) 
	series->append ((qreal)i, (qreal)get_real (res, i));
      lcl_chart->addSeries (series);
      lcl_chart->createDefaultAxes ();
      QString cmd = QString (")erase %1").arg (expvar).arg (xlbl);
      apl_command (cmd.toStdString ().c_str ());
      if (zset) {
	cmd = QString (")erase %1").arg (zlbl);
	apl_command (cmd.toStdString ().c_str ());
      }
    }
  }
}

void
MainWindow::setValue ()
{
  lcl_chart->removeAllSeries();
  QLineSeries *series = new QLineSeries ();

  qreal x;
  switch (use_func) {
  case USE_COS:
    for (x = 0.0; x < 6.28; x += 0.1) series->append (x, 2.0*qCos (x));
    lcl_chart->setTitle ("Cosine curve");
    break;
  case USE_SIN:
    for (x = 0.0; x < 6.28; x += 0.1) series->append (x, qSin (x));
    lcl_chart->setTitle ("Sine curve");
    break;
  }
  if (++use_func >= USE_MAX) use_func = 0;
      
  lcl_chart->addSeries (series);
  lcl_chart->createDefaultAxes ();
}

void
MainWindow::buildMenu (MainWindow *win)
{
  QGroupBox *formGroupBox = new QGroupBox ("Visualisation");
  QGridLayout *layout = new QGridLayout;


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

  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  layout->addWidget (x_var_name, 0, 0);
  
  x_var_min = new  QLineEdit ();
  x_var_min->setPlaceholderText ("x minimum value");
  layout->addWidget (x_var_min, 0, 1);

  x_var_max = new  QLineEdit ();
  x_var_max->setPlaceholderText ("x maximum value");
  layout->addWidget (x_var_max, 0, 2);

  /*  z indep vbl */

  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  layout->addWidget (z_var_name, 1, 0);

  z_var_min = new  QLineEdit ();
  z_var_min->setPlaceholderText ("z minimum value");
  layout->addWidget (z_var_min, 1, 1);

  z_var_max = new  QLineEdit ();
  x_var_max->setPlaceholderText ("z maximum value");
  layout->addWidget (z_var_max, 1, 1);


  /*  APL expression */

  apl_expression = new  QLineEdit ();
  layout->addWidget (apl_expression, 2, 0);

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
  use_func = USE_SIN;
  lcl_chart = chartView->chart ();

  buildMenu (this);

  setValue ();
}

MainWindow::~MainWindow()
{

}

