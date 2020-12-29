#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"
#include "ui_mainwindow.h"

// what gives

void MainWindow::setValue()
{
    lcl_chart->removeAllSeries();
    QLineSeries *series = new QLineSeries();

    qreal x;
    for (x = 0.0; x < 6.28; x += 0.1) series->append(x, qCos(x));
    lcl_chart->addSeries(series);
}

MainWindow::MainWindow(QChartView *chartView, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    lcl_chart = chartView->chart();

    QGroupBox *formGroupBox = new QGroupBox("Depth of Field");
    QGridLayout *layout = new QGridLayout;

    QString compute_button_style("background-color: yellow; color: red;");
    QFont   compute_button_font("bold");
    QPushButton *compute_button = new QPushButton(QObject::tr("Compute f stop"));
    compute_button->setStyleSheet (compute_button_style);
    compute_button->setFont(compute_button_font);
    compute_button->setToolTip ("Compute f-stop");
    layout->addWidget(compute_button, 0, 0);

    QObject::connect(compute_button,
                         SIGNAL(clicked ()),
                         this,
                         SLOT(setValue()));

    formGroupBox->setLayout(layout);
    formGroupBox->show();
    QLineSeries *series = new QLineSeries();

    qreal x;
    for (x = 0.0; x < 6.28; x += 0.1) series->append(x, qSin(x));

    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");

    chartView->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

