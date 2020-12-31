#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

#define USE_SIN 0
#define USE_COS 1
#define USE_MAX 2

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QChartView *chartView, QWidget *parent = nullptr);
  ~MainWindow();
  void buildMenu (MainWindow *win);

public slots:

private slots:
  void handleExpression ();
  void setValue ();
  
private:
  int use_func;
  QChart *lcl_chart;
  QLineEdit *x_var_name;
  QLineEdit *x_var_min;
  QLineEdit *x_var_max;
  QLineEdit *z_var_name;
  QLineEdit *z_var_min;
  QLineEdit *z_var_max;
  QLineEdit *apl_expression;
  QSettings settings;
};
#endif // MAINWINDOW_H
