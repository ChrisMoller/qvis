#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"

void
MainWindow::handleExpression ()
{
  QString input = apl_expression->text ();
  qInfo ("handling exp %s\n", input.toStdString ().c_str ());
  apl_exec(input.toStdString ().c_str ());
  APL_value a = get_var_value ("a", "something");
  qInfo ("rank  = %d\n", get_rank (a));
  uint64_t count = get_element_count (a);
  qInfo ("a = [%d] ", (int)count);
  for (uint64_t i = 0; i < count; i++)
    qInfo (" %d ", (int)get_int (a, i));
  qInfo ("\n");
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
  QGroupBox *formGroupBox = new QGroupBox ("Depth of Field");
  QGridLayout *layout = new QGridLayout;


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


  /*  APL expression */

  apl_expression = new  QLineEdit ();
  layout->addWidget (apl_expression, 0, 1);

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
  layout->addWidget (quit_button, 1, 1);

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

