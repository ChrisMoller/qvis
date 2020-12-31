#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

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
  
private:
  QChartView *lcl_chartView;
  QChart *lcl_chart;
  QLineEdit *chart_title;
  QLineEdit *y_title;
  QLineEdit *x_title;
  QLineEdit *x_var_name;
  QLineEdit *x_var_min;
  QLineEdit *x_var_max;
  QLineEdit *z_title;
  QLineEdit *z_var_name;
  QLineEdit *z_var_min;
  QLineEdit *z_var_max;
  QLineEdit *apl_expression;
  QLineEdit *fcn_label;
  QSettings settings;
};
#endif // MAINWINDOW_H
