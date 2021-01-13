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

// https://doc.qt.io/qt-5/qtdatavisualization-index.html

#include <QtWidgets>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QPolarChart>
#include <QMenuBar>
#include <values.h>

#include <iostream>
#include <sstream>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"
#include "chartwindow.h"
#include "xml.h"

#define expvar "expvarλ"

void
MainWindow::closeEvent(QCloseEvent *event __attribute__((unused)))
{
  byebye ();
}

void
MainWindow::themeChanged (int newtheme __attribute__((unused)))
{
  theme = (QChart::ChartTheme)themebox->currentData ().toInt ();
  chartWindow->handleExpression ();
}

bool
MainWindow::maybeSave()
{
  bool rc = true;	// true => ok to zap, false => don't zap;
  if (changed) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Scribble"),
			       tr("The curve may have been modified.\n"
				  "Do you want to save your changes?"),
			       QMessageBox::Save | QMessageBox::Discard
			       | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) save();
    else if (ret == QMessageBox::Cancel) rc = false;
  }
  return rc;
}


void
MainWindow::byebye ()
{
  if (maybeSave ()) {
    chartWindow->saveSettings ();
    QCoreApplication::quit ();
  }
}

#if 0
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
  changed = true;
}
#endif

void
MainWindow::handleExpression ()
{
  chartWindow->handleExpression ();
}

void
MainWindow::valChanged (bool enabled __attribute__((unused)))
{
  chartWindow->curve.polar  = (Qt::Checked == do_polar->checkState());
  chartWindow->curve.spline = (Qt::Checked == do_spline->checkState());
  //  curve.shorttitle
  chartWindow->curve.title			= chart_title->text ();
  chartWindow->curve.function.label		= y_title->text ();
  chartWindow->curve.function.title		= fcn_label->text ();
  chartWindow->curve.function.expression	= apl_expression->text ();
  chartWindow->curve.ix.name			= x_var_name->text ();
  chartWindow->curve.ix.title		= x_title->text ();
  chartWindow->curve.ix.range.min		= x_var_min->value ();
  chartWindow->curve.ix.range.max		= x_var_max->value ();
  chartWindow->curve.iz.name			= z_var_name->text ();
  chartWindow->curve.iz.title		= z_title->text ();
  chartWindow->curve.iz.range.min 		= z_var_min->value ();
  chartWindow->curve.iz.range.max 		= z_var_max->value ();
  chartWindow->changed = true;
  chartWindow->handleExpression ();
}

void
MainWindow::valChangedv ()
{
  valChanged (true);
}

void
MainWindow::newFile()
{
  MainWindow window (nullptr);
#if 0
  chartView = new QChartView ();
  chartView->setChart (curve.polar ? polarchart : chart);
  chartView->setRenderHint (QPainter::Antialiasing);
  handleExpression ();
#endif
  
#if 0
  this->setCentralWidget (newChartView);
  if (maybeSave()) {
    textEdit->clear();
    setCurrentFile(QString());
  }
#endif
}

void
MainWindow::open()
{
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setNameFilter("*.vis");
  if (dialog.exec() == QDialog::Accepted)
    chartWindow->readFile(dialog.selectedFiles().first());
}

bool
MainWindow::save()
{
  if (curFile.isEmpty()) {
    return saveAs();
  } else {
    return chartWindow->saveFile(curFile);
  }
}

bool
MainWindow::saveAs()
{
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted)
    return false;
  return chartWindow->saveFile(dialog.selectedFiles().first());
}

void
MainWindow::about()
{
  QMessageBox::about(this,
		     tr("About qvis"),
	     tr("<b>qvis</b> allows APL expressions to be interactively "
		"visualised with respect axes minima and maxima and"
                "dynamically variable parameters."));
}

void
MainWindow::create_menuBar ()
{
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QToolBar *fileToolBar = addToolBar(tr("File"));
  const QIcon newIcon =
    QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
  QAction *newAct = new QAction(newIcon, tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
  fileMenu->addAction(newAct);
  fileToolBar->addAction(newAct);

  const QIcon openIcon =
    QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
  QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(openAct);
  fileToolBar->addAction(openAct);

  const QIcon saveIcon =
    QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(saveAct);
  fileToolBar->addAction(saveAct);

  const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
  QAction *saveAsAct =
    fileMenu->addAction(saveAsIcon, tr("Save &As..."), this,
			&MainWindow::saveAs);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));

  fileMenu->addSeparator();

  const QIcon exitIcon =
    QIcon::fromTheme("application-exit",
		     QIcon(":/images/application-exit.png"));
  QAction *exitAct =
    fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  fileToolBar->addAction(exitAct);

#if 0
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QToolBar *editToolBar = addToolBar(tr("Edit"));
#endif

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAct =
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction *aboutQtAct =
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::returnPressed()
{
  QString text = aplline->text();
  aplline->setText ("");
  aplwin->append (text);
  
  std::stringstream outbuffer;
  std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(outbuffer.rdbuf());

  std::stringstream errbuffer;
  std::streambuf *cerrbuf = std::cerr.rdbuf();
  std::cerr.rdbuf(errbuffer.rdbuf());
  
  LIBAPL_error execerr = apl_exec (text.toStdString ().c_str ());
  if (execerr != LAE_NO_ERROR) {
    QString emsg =
      QString ("APL error %1").arg ((int)execerr, 8, 16, QLatin1Char('0'));
    aplwin->append (emsg);
    if (errbuffer.str ().size () > 0)
      aplwin->append (errbuffer.str ().c_str ());
  }

  std::cout.rdbuf(coutbuf);
  std::cerr.rdbuf(cerrbuf);

  if (outbuffer.str ().size () > 0)
    aplwin->append (outbuffer.str ().c_str ());
}

bool
KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
  static int rc = 0;
  if (obj == watched) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Up) {
	fprintf (stderr, "kpf %d up\n", rc++);
	return true;
      }
      else if(keyEvent->key() == Qt::Key_Down) {
	fprintf (stderr, "kpf %d down\n", rc++);
	return true;
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

void
MainWindow::buildMenu ()
{
  create_menuBar ();
  QGroupBox *formGroupBox = new QGroupBox ();
  QGridLayout *layout = new QGridLayout;

  QSettings settings;

  int row = 0;
  int col = 0;

  aplwin = new QTextEdit ();
  aplwin->setReadOnly (true);
  layout->addWidget (aplwin, row, 0, 1, 4);

  row++;
  aplline = new  QLineEdit ();
  aplline->setPlaceholderText ("APL");
  keyPressEater = new KeyPressEater (aplline);
  aplline->installEventFilter(keyPressEater);
  connect(aplline, &QLineEdit::returnPressed,
	  this, &MainWindow::returnPressed);
  layout->addWidget (aplline, row, 0, 1, 4);
  
  row++;
  col = 0;

  chartWindow->curve.title = settings.value (CHART_TITLE).toString ();
  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  chart_title->setText (chartWindow->curve.title);
  layout->addWidget (chart_title, row, 0, 1, 3);
  
  chartWindow->curve.function.label = settings.value (Y_TITLE).toString ();
  y_title = new  QLineEdit ();
  y_title->setPlaceholderText ("y axix label");
  y_title->setText (chartWindow->curve.function.label);
  layout->addWidget (y_title, row, 3);
  
  /*  x indep vbl */

  row++;
  col = 0;
  
  chartWindow->curve.ix.name = settings.value (X_VAR_NAME).toString ();
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  x_var_name->setText (chartWindow->curve.ix.name);
  layout->addWidget (x_var_name, row, col++);
  
  chartWindow->curve.ix.range.min = settings.value (X_VAR_MIN).toDouble ();
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_min->setToolTip ("x minimum value");
  x_var_min->setValue (chartWindow->curve.ix.range.min);
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_min, row, col++);

  chartWindow->curve.ix.range.max = settings.value (X_VAR_MAX).toDouble ();
  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_max->setToolTip ("x maximum value");
  x_var_max->setValue (chartWindow->curve.ix.range.max);
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_max, row, col++);
  
  chartWindow->curve.ix.title = settings.value (X_TITLE).toString ();
  x_title = new  QLineEdit ();
  x_title->setPlaceholderText ("x axix label");
  x_title->setText (chartWindow->curve.ix.title);
  layout->addWidget (x_title, row, col++);

  /*  z indep vbl */

  row++;
  col = 0;

  chartWindow->curve.iz.name = settings.value (Z_VAR_NAME).toString ();
  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  z_var_name->setText (chartWindow->curve.iz.name);
  layout->addWidget (z_var_name, row, col++);

  chartWindow->curve.iz.range.min = settings.value (Z_VAR_MIN).toDouble ();
  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_min->setToolTip ("z minimum value");
  z_var_min->setValue (chartWindow->curve.iz.range.min);
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_min, row, col++);

  chartWindow->curve.iz.range.max = settings.value (Z_VAR_MAX).toDouble ();
  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_max->setToolTip ("z maximum value");
  z_var_max->setValue (chartWindow->curve.iz.range.max);
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_max, row, col++);
  
  chartWindow->curve.iz.title = settings.value (Z_TITLE).toString ();
  z_title = new  QLineEdit ();
  z_title->setPlaceholderText ("z axix label");
  z_title->setText (chartWindow->curve.iz.title);
  layout->addWidget (z_title, row, col++);


  /*  APL expression */

  row++;
  col = 0;
  chartWindow->curve.function.title = settings.value (FCN_LABEL).toString ();
  fcn_label = new  QLineEdit ();
  fcn_label->setPlaceholderText ("curve label");
  fcn_label->setText (chartWindow->curve.function.title);
  layout->addWidget (fcn_label, row, col++);

  chartWindow->curve.function.expression =
    settings.value (FUNCTION).toString ();
  apl_expression = new  QLineEdit ();
  apl_expression->setPlaceholderText ("function");
  apl_expression->setText (chartWindow->curve.function.expression);
  layout->addWidget (apl_expression, row, col, 1, 3);
  QObject::connect (apl_expression,
		    &QLineEdit::editingFinished,
		    this,
		    &MainWindow::valChangedv);

  /*  toggles */
  
  row++;
  col = 0;
  
  chartWindow->curve.spline = settings.value (DO_SPLINE).toBool ();
  do_spline = new QCheckBox ("Spline");
  do_spline->setCheckState (chartWindow->curve.spline
			    ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);
  
  chartWindow->curve.polar = settings.value (DO_POLAR).toBool ();
  do_polar = new QCheckBox ("Polar");
  do_polar->setCheckState (chartWindow->curve.polar
			   ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);


  /*   buttons */

  /*   settings button   */

#if 0
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

#endif


  /*   end buttons  */

  formGroupBox->setLayout (layout);
  formGroupBox->show ();
  this->setCentralWidget (formGroupBox);
  chartWindow->handleExpression ();
}

MainWindow::MainWindow (QWidget *parent)
  : QMainWindow(parent)
{
  chartWindow = new ChartWindow (this);

  buildMenu ();

  this->show ();

  changed = false;
}

MainWindow::~MainWindow()
{
}

