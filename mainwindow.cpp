#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"

void
MainWindow::handleExpression ()
{
  QString xlbl = x_var_name->text ();
  QString xmin = x_var_min->text ();
  QString xmax = x_var_max->text ();
#if 0
  QString zlbl = z_var_name->text ();
  QString zmin = z_var_min->text ();
  QString zmax = z_var_max->text ();
#endif

  /***
      lbl ← min + (⍳incr) * (max - min) / incr
   ***/
  QString cmd1 = xlbl + "←" + "(⍳16)" + "(" + xmax + "-" + xmin + ")" + "÷16";
  apl_exec (cmd1.toStdString ().c_str ());
  
  QString input = apl_expression->text ();
  apl_exec(input.toStdString ().c_str ());

  APL_value a = get_var_value ("a", "something");
  if (a) {
    lcl_chart->removeAllSeries();
    QLineSeries *series = new QLineSeries ();
    uint64_t count = get_element_count (a);
    for (uint64_t i = 0; i < count; i++) 
      series->append ((qreal)i, (qreal)get_real (a, i));
    lcl_chart->addSeries (series);
    lcl_chart->createDefaultAxes ();
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

