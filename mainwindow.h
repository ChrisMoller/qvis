#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QChartView *chartView, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
  void setValue ();

private:
    QChart *lcl_chart;
};
#endif // MAINWINDOW_H
