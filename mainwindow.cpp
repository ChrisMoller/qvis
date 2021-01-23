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
#include "history.h"
#include "aplexec.h"
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
  if (maybeSave ()) QCoreApplication::quit ();
}

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
  chartWindow->curve.function.title		= y_title->text ();
  chartWindow->curve.function.label		= fcn_label->text ();
  chartWindow->curve.function.expression	= apl_expression->text ();
  chartWindow->curve.ix.name			= x_var_name->text ();
  chartWindow->curve.ix.title			= x_title->text ();
  chartWindow->curve.ix.range.min		= x_var_min->value ();
  chartWindow->curve.ix.range.max		= x_var_max->value ();
  chartWindow->curve.iz.name			= z_var_name->text ();
  chartWindow->curve.iz.title			= z_title->text ();
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
  QString     emptystring;
  QStringList emptylist;
  MainWindow window (emptystring, emptylist, emptystring);
}

void
MainWindow::open()
{
  ChartWindow *newchartWindow = new ChartWindow (this);
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  dialog.setNameFilter("*.vis");
  if (dialog.exec() == QDialog::Accepted)
    newchartWindow->readFile(dialog.selectedFiles().first());
  chartWindow = newchartWindow;
}

void
MainWindow::loadapl(bool copy)
{
  /***
      xml )load )copy
      atf  )in
      
      apl not work
      xml works
      atf works but no msg
      
   ***/

  fprintf (stderr, "copy = %d\n", copy);
  QString filter = copy ? QString ("*.xml") :  QString ("*.xml *.atf");
  QFileDialog dialog(this, QString ("Open APL file"), libpath, filter);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  QString outString;
  QString errString;
  if (dialog.exec() == QDialog::Accepted) {
    QString fn =  dialog.selectedFiles().first();
    if (fn.endsWith (QString (".xml"),Qt::CaseInsensitive)) {
      QString op = copy ? QString (")copy") : QString (")load");
      QString cmd = QString ("%1 %2").arg (op).arg (fn);
      AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
    }
    else if (!copy && fn.endsWith (QString (".atf"),Qt::CaseInsensitive)) {
      QString cmd = QString (")in %1").arg (fn);
      AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
    }
    else {
      QMessageBox msgBox;
      msgBox.setText("File type not supported.");
      msgBox.setIcon (QMessageBox::Warning);
      msgBox.exec();
    }
  }
  if (!errString.isEmpty ()) {
    aplwin->setTextColor (QColor (255, 0, 0));
    aplwin->setText (outString);
    aplwin->setTextColor (QColor (0, 0, 0));
  }
  if (!outString.isEmpty ()) aplwin->setText (outString);
}

void
MainWindow::copyapl()
{
  loadapl(true);
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
#ifdef USE_TOOLBAR
  QToolBar *fileToolBar = addToolBar(tr("File"));
#endif
  
  const QIcon newIcon =
    QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
  QAction *newAct = new QAction(newIcon, tr("&New"), this);
#ifdef USE_TOOLBAR
  fileToolBar->addAction(newAct);
#endif
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
  fileMenu->addAction(newAct);

  const QIcon openIcon =
    QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
  QAction *loadAct = new QAction(openIcon, tr("&Load..."), this);
#ifdef USE_TOOLBAR
  fileToolBar->addAction(loadAct);
#endif
  loadAct->setShortcuts(QKeySequence::Open);
  loadAct->setStatusTip(tr("Load an existing workspace"));
  connect(loadAct, &QAction::triggered, this, &MainWindow::loadapl);
  fileMenu->addAction(loadAct);

  QAction *copyAct = new QAction(openIcon, tr("&Copy..."), this);
#ifdef USE_TOOLBAR
  fileToolBar->addAction(copyAct);
#endif
  //  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy an existing workspace"));
  connect(copyAct, &QAction::triggered, this, &MainWindow::copyapl);
  fileMenu->addAction(copyAct);

  const QIcon saveIcon =
    QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
#ifdef USE_TOOLBAR
  fileToolBar->addAction(saveAct);
#endif
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(saveAct);

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
#ifdef USE_TOOLBAR
  fileToolBar->addAction(exitAct);
#endif

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAct =
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction *aboutQtAct =
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

static const QColor red = QColor (255, 0, 0);
static const QColor black = QColor (0, 0, 0);

void
MainWindow::process_line(QString text)
{
  QColor red = QColor (255, 0, 0);
  QColor black = QColor (0, 0, 0);
  aplline->setText ("");
  
  aplwin->append (text);

  QString outString;
  QString errString;
  LIBAPL_error rc = AplExec::aplExec (APL_OP_EXEC, text,outString, errString);

  if (rc != LAE_NO_ERROR) {
    QString emsg =
      QString ("APL error %1").arg ((int)rc, 8, 16, QLatin1Char('0'));
    aplwin->setTextColor (red);
    aplwin->append (emsg);
    if (errString.size () > 0)
      aplwin->append (errString);
    aplwin->setTextColor (black);
  }
  
  if (outString.size () > 0)
    aplwin->append (outString);
}

void MainWindow::returnPressed()
{
  QString text = aplline->text();
  history->insert (text.toStdString ().c_str ());
  process_line (text);
  history->rebase ();
}

bool
KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == watched) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Up) {
	char *str = mainwin->history->previous ();
	QString text = QString (str);
	mainwin->aplline->setText (text);
	return true;
      }
      else if(keyEvent->key() == Qt::Key_Down) {
	char *str = mainwin->history->next ();
	QString text = QString (str);
	mainwin->aplline->setText (text);
	return true;
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

void
MainWindow::buildMenu (QString &msgs)
{
  create_menuBar ();

  QGroupBox *outerGroupBox = new QGroupBox ();
  QVBoxLayout *outerlayout = new QVBoxLayout ();
  outerGroupBox->setLayout (outerlayout);
  
  {
    QGroupBox *formGroupBox = new QGroupBox (QString ("Sandbox"));
    QVBoxLayout *layout = new QVBoxLayout ();

    aplwin = new QTextEdit ();
    aplwin->setReadOnly (true);
    if (!msgs.isEmpty ()) aplwin->setText (msgs);
    layout->addWidget (aplwin);

    aplline = new  QLineEdit ();
    aplline->setPlaceholderText ("APL");
    keyPressEater = new KeyPressEater (aplline, this);
    aplline->installEventFilter(keyPressEater);
    connect(aplline, &QLineEdit::returnPressed,
	    this, &MainWindow::returnPressed);
    layout->addWidget (aplline);

    formGroupBox->setLayout (layout);
    formGroupBox->setAlignment (Qt::AlignLeft);
    outerlayout->addWidget (formGroupBox);
  }
  {
    QGroupBox *formGroupBox = new QGroupBox (QString ("Chart control"));
    QGridLayout *layout = new QGridLayout;;
    QMenuBar *mb2 = new QMenuBar ();
    QMenu *fileMenu = mb2->addMenu(tr("&File"));
    QAction *openAct = new QAction(tr("&Open Chart..."), this);
    openAct->setStatusTip(tr("Open an existing vis file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    layout->setMenuBar (mb2);

    int row = 0;
    int col = 0;

    chart_title = new  QLineEdit ();
    chart_title->setPlaceholderText ("chart title");
    layout->addWidget (chart_title, row, 0, 1, 3);

    y_title = new  QLineEdit ();
    y_title->setPlaceholderText ("y axix label");
    layout->addWidget (y_title, row, 3);


    row++;
    col = 0;

    /*  x indep vbl */
  
    x_var_name = new  QLineEdit ();
    x_var_name->setPlaceholderText ("x variable name");
    layout->addWidget (x_var_name, row, col++);
  
    x_var_min = new  QDoubleSpinBox ();
    x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
    x_var_min->setToolTip ("x minimum value");
    QObject::connect (x_var_min,
		      QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		      this,
		      &MainWindow::valChanged);
    layout->addWidget (x_var_min, row, col++);

    x_var_max = new  QDoubleSpinBox ();
    x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
    x_var_max->setToolTip ("x maximum value");
    QObject::connect (x_var_max,
		      QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		      this,
		      &MainWindow::valChanged);
    layout->addWidget (x_var_max, row, col++);
  
    x_title = new  QLineEdit ();
    x_title->setPlaceholderText ("x axix label");
    layout->addWidget (x_title, row, col++);

    /*  z indep vbl */

    row++;
    col = 0;

    z_var_name = new  QLineEdit ();
    z_var_name->setPlaceholderText ("z variable name");
    layout->addWidget (z_var_name, row, col++);

    z_var_min = new  QDoubleSpinBox ();
    z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
    z_var_min->setToolTip ("z minimum value");
    QObject::connect (z_var_min,
		      QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		      this,
		      &MainWindow::valChanged);
    layout->addWidget (z_var_min, row, col++);

    z_var_max = new  QDoubleSpinBox ();
    z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
    z_var_max->setToolTip ("z maximum value");
    QObject::connect (z_var_max,
		      QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		      this,
		      &MainWindow::valChanged);
    layout->addWidget (z_var_max, row, col++);
  
    z_title = new  QLineEdit ();
    z_title->setPlaceholderText ("z axix label");
    layout->addWidget (z_title, row, col++);
    
    /*  APL expression */

    row++;
    col = 0;

    fcn_label = new  QLineEdit ();
    fcn_label->setPlaceholderText ("curve label");
    layout->addWidget (fcn_label, row, col++);

    apl_expression = new  QLineEdit ();
    apl_expression->setPlaceholderText ("function");
    layout->addWidget (apl_expression, row, col, 1, 3);
    QObject::connect (apl_expression,
		      &QLineEdit::editingFinished,
		      this,
		      &MainWindow::valChangedv);

    /*  toggles */
  
    row++;
    col = 0;
  
    do_spline = new QCheckBox ("Spline");
    layout->addWidget (do_spline, row, col++);
    connect(do_spline,
	    &QCheckBox::stateChanged,
	    this,
	    &MainWindow::valChanged);
  
    do_polar = new QCheckBox ("Polar");
    layout->addWidget (do_polar, row, col++);
    connect(do_polar,
	    &QCheckBox::stateChanged,
	    this,
	    &MainWindow::valChanged);
    
    formGroupBox->setLayout (layout);
    formGroupBox->setAlignment (Qt::AlignLeft);
    outerlayout->addWidget (formGroupBox);
  }
  outerGroupBox->show ();
  
  this->setCentralWidget (outerGroupBox);
}

void
MainWindow::enterChart (ChartWindow *cw)
{
  chart_title->setText (cw->curve.title);
  y_title->setText (cw->curve.function.label);

  x_var_name->setText (cw->curve.ix.name);
  x_var_min->setValue (cw->curve.ix.range.min);
  x_var_max->setValue (cw->curve.ix.range.max);
  x_title->setText (cw->curve.ix.title);

  z_var_name->setText (cw->curve.iz.name);
  z_var_min->setValue (cw->curve.iz.range.min);
  z_var_max->setValue (cw->curve.iz.range.max);
  z_title->setText (cw->curve.iz.title);

  fcn_label->setText (chartWindow->curve.function.title);
  apl_expression->setText (cw->curve.function.expression);

  do_spline->setCheckState (cw->curve.spline ? Qt::Checked : Qt::Unchecked);
  do_polar->setCheckState (cw->curve.polar   ? Qt::Checked : Qt::Unchecked);
  chartWindow = cw;
}

MainWindow::MainWindow (QString &msgs, QStringList &args,
			QString &lp, QWidget *parent)
  : QMainWindow(parent)
{
  history = new History ();
  libpath = lp;

  if (!args.empty ()) {
    int i;
    for (i = 0; i < args.count (); i++) {
      ChartWindow *newchartWindow = new ChartWindow (this);
      QString fn = args.value (i);
      newchartWindow->readFile (fn);
      chartWindow = newchartWindow;
    }
  }
  
  buildMenu (msgs);

  if (!args.empty ()) {
    int i;
    for (i = 0; i < args.count (); i++) {
      enterChart (chartWindow);
    }
  }
  
  this->show ();

  changed = false;
}

MainWindow::~MainWindow()
{
}

