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

// /old_home/Qt/Examples/Qt-5.15.1/opengl/qopenglwindow/background_renderer.cpp

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
#include <QProcess>
#include <QInputDialog>
#include <values.h>

#include <iostream>
#include <sstream>

#include <sys/stat.h>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"
#include "chartwindow.h"
#include "history.h"
#include "aplexec.h"
#include "xml.h"

#define expvar "expvarλ"
#define LAMBDA_HEADER "lambda_"

static const QColor red = QColor (255, 0, 0);
static const QColor black = QColor (0, 0, 0);

// these are all known to work
// #define DEFAULT_EDITOR "emacs -f \"global-display-line-numbers-mode\""
// #define DEFAULT_EDITOR "gvim \"+set number\""
#define DEFAULT_EDITOR "gvim -c \"set nu\""

void
MainWindow::update_screen (QString &errString, QString &outString)
{
  if (!errString.isEmpty ()) {
    aplwin->setTextColor (red);
    aplwin->append (outString);
    aplwin->setTextColor (black);
  }
  if (!outString.isEmpty ()) aplwin->append (outString);
}

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

#if 0
void
MainWindow::newFile()
{
}
#endif

#if 1
void
MainWindow::gvimDone (int something)
{
  fprintf (stderr, "gvimDone %d\n", something);
}

void
MainWindow::gvimErr (QProcess::ProcessError error)
{
  fprintf (stderr, "gvimErr %d\n", (int)error);
}
#endif

void
MainWindow::fileChanged(const QString &path)
{
  QFile file(path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString outString;
    QString errString;
    QTextStream in(&file);
    QStringList fcn;
    int len = 0;
    while (!in.atEnd()) {
      QString line = in.readLine();
      if (len < line.size ()) len = line.size ();
      fcn += line;
    }
    
    if (path.contains (LAMBDA_HEADER)) {
      if (fcn.size () == 1) {
	QFileInfo info (file);
	QString name = info.baseName ();
	name.remove (0, QString (LAMBDA_HEADER).size ());
	QString stmt = fcn.last ().trimmed ();
	QString cmd = QString (")erase %1").arg (name);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	cmd = QString ("%1←{%2}").arg (name).arg (stmt);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	update_screen (errString, outString);
      }
      else {
	//fixme invalid lambda
	return;
      }
    }
    else {
      int i;
      QString mtx;
      for (i = 0; i < fcn.size (); i++) {
	QString ln = fcn[i];
	ln.resize (len, QChar (' '));
	mtx.append (ln);
      }
      APL_value aplv = char_vector (toCString (mtx), "qvis");
      set_var_value (expvar, aplv, "qvis");
      QString cmd = QString ("%1←%2 %3ρ%4")
	.arg (expvar).arg (fcn.size ()).arg (len).arg (expvar);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
      cmd = QString ("⎕fx %1").arg (expvar);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
      update_screen (errString, outString);
      release_value (aplv, "qvis");
      cmd = QString (")erase %1").arg (expvar);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
    }
  }
}

#if 0
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
#endif

void
MainWindow::loadapl()
{
  /***
      xml )load )copy
      atf  )in
      
      apl not work
      xml works
      atf works but no msg
      
   ***/

  static bool protect = false;
  static bool do_load = true;
  QString filter = QString ("*.xml *.atf");
  QFileDialog dialog(this, QString ("Open APL file"), libpath, filter);
  dialog.setOption (QFileDialog::DontUseNativeDialog);
  QLayout *layout = dialog.layout ();

  QGroupBox *gbox = new QGroupBox ();
  QHBoxLayout *btnlayout = new QHBoxLayout ();
  gbox->setLayout (btnlayout);
  QCheckBox *button_protected  = new QCheckBox ("Protected", this);
  QRadioButton *button_load    = new QRadioButton ("Load", this);
  QRadioButton *button_copy    = new QRadioButton ("Copy", this);
  button_load->setChecked (do_load);
  btnlayout->addWidget (button_protected);
  btnlayout->addWidget (button_load);
  btnlayout->addWidget (button_copy);
  layout->addWidget (gbox);
  
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptOpen);
  QString outString;
  QString errString;
  if (dialog.exec() == QDialog::Accepted) {
    do_load = button_load->isChecked();
    protect =
      (button_protected->checkState() == Qt::Checked) ? true : false;
    QString fn =  dialog.selectedFiles().first();
    if (fn.endsWith (QString (".xml"),Qt::CaseInsensitive)) {
      if (do_load && protect) {
	QMessageBox msgBox;
  msgBox.setText("Loaded workspaces cannot be protected.  Use )copy instead..");
	msgBox.setIcon (QMessageBox::Warning);
	msgBox.exec();
      }
      else {
	QString op =
	  do_load
	  ? QString (")load")
	  : (protect ? QString (")pcopy") : QString (")copy"));
	QString cmd = QString ("%1 %2").arg (op).arg (fn);
	AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
      }
    }
    else if (fn.endsWith (QString (".atf"),Qt::CaseInsensitive)) {
      QString op =
	protect ? QString (")pin") : QString (")in");
      QString cmd = QString ("%1 %2").arg(op).arg (fn);
      AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
    }
    else {
      QMessageBox msgBox;
      msgBox.setText("File type not supported.");
      msgBox.setIcon (QMessageBox::Warning);
      msgBox.exec();
    }
  }
  update_screen (errString, outString);
  delete gbox;
}

#if 0
void
MainWindow::copyapl()
{
  openapl(true);
}

void
MainWindow::loadapl()
{
  openapl(false);
}
#endif

bool
MainWindow::save()
{
/***
    )save and )dump need wsid or argument
    )out always needs argument
 ***/
  bool rc = false;
  QString op;
  switch(save_mode) {
  case SAVE_MODE_NONE:
    break;
  case SAVE_MODE_SAVE:
    op = QString (")save");
    break;
  case SAVE_MODE_DUMP:
    op = QString (")save");
    break;
  case SAVE_MODE_OUT:
    break;
  }
  if (!op.isEmpty ()) {
    QString outString;
    QString errString;
    AplExec::aplExec (APL_OP_EXEC, op, outString, errString);
    update_screen (errString, outString);
    rc = true;
  }
  return rc;
}

bool
MainWindow::saveAs()
{
  bool rc = false;
  QFileDialog dialog(this);
  dialog.setOption (QFileDialog::DontUseNativeDialog);
  QLayout *layout = dialog.layout ();
  QGroupBox *gbox = new QGroupBox ("Save mode");
  QHBoxLayout *btnlayout = new QHBoxLayout ();
  gbox->setLayout (btnlayout);
  QRadioButton *button_save = new QRadioButton("Save", this);
  QRadioButton *button_dump = new QRadioButton("Dump", this);
  QRadioButton *button_out  = new QRadioButton("Out", this);
  btnlayout->addWidget (button_save);
  btnlayout->addWidget (button_dump);
  btnlayout->addWidget (button_out);
  switch (save_mode) {
  case  SAVE_MODE_NONE:
    break;
  case  SAVE_MODE_SAVE:
    button_save->setChecked (true);
    break;
  case  SAVE_MODE_DUMP:
    button_dump->setChecked (true);
    break;
  case  SAVE_MODE_OUT:
    button_out->setChecked (true);
    break;
  }
  layout->addWidget (gbox);
  //  gbox->show ();
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  int drc = dialog.exec();
  delete gbox;
  if (drc == QDialog::Accepted) {
    QString op;
    curFile = dialog.selectedFiles().first();
    if (button_save->isChecked ()) {
      if (!curFile.endsWith (".xml", Qt::CaseInsensitive))
	curFile.append (".xml");
      save_mode = SAVE_MODE_SAVE;
      op = QString (")save");
    }
    else if (button_dump->isChecked ()) {
      save_mode = SAVE_MODE_DUMP;
      if (!curFile.endsWith (".apl", Qt::CaseInsensitive))
	curFile.append (".apl");
      op = QString (")dump");
    }
    else if (button_out->isChecked ()) {
      save_mode = SAVE_MODE_OUT;
      if (!curFile.endsWith (".atf", Qt::CaseInsensitive))
	curFile.append (".atf");
      op = QString (")out");
    }
    if (!op.isEmpty ()) {
      QString outString;
      QString errString;
      QString cmd = QString ("%1 %2").arg (op).arg (curFile);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
      update_screen (errString, outString);
      rc = true;
    }
  }
  return rc;
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
MainWindow::setGeneral ()
{
  QDialog dialog (this, Qt::Popup);
  QGridLayout *layout = new QGridLayout;
  dialog.setLayout (layout);

  int row = 0;
  QLabel *editorLabel = new QLabel(QString ("Editor"), this);
  layout->addWidget (editorLabel, row, 0);
  QLineEdit *editorSelect = new  QLineEdit ();
  if (!editor.isEmpty ()) editorSelect->setText (editor);
  editorSelect->setPlaceholderText ("APL");
  layout->addWidget (editorSelect, row, 1);

  row++;
  QPushButton *closeButton = new QPushButton (QObject::tr ("Close"));
  layout->addWidget (closeButton, row, 1);
  QObject::connect (closeButton, &QPushButton::clicked,
		    &dialog, &QDialog::accept);
  QPushButton *cancelButton = new QPushButton (QObject::tr ("Cancel"));
  layout->addWidget (cancelButton, row, 0);
  QObject::connect (cancelButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);

  QPoint loc = this->pos ();
  dialog.move (loc.x () + 200, loc.y () + 200);
  int drc = dialog.exec ();
  if (drc == QDialog::Accepted) {
    if (!editorSelect->text ().isEmpty ()) {
      QSettings settings;
      editor = editorSelect->text ();
      settings.setValue (QString (SETTINGS_EDITOR), QVariant (editor));
    }
  }
  delete editorLabel;
  delete editorSelect;
  delete closeButton;
  delete layout;
}

void
MainWindow::create_menuBar ()
{
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
#ifdef USE_TOOLBAR
  QToolBar *fileToolBar = addToolBar(tr("File"));
#endif

#if 0
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
#endif

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

#if 0
  QAction *copyAct = new QAction(openIcon, tr("&Copy..."), this);
#ifdef USE_TOOLBAR
  fileToolBar->addAction(copyAct);
#endif
  //  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy an existing workspace"));
  connect(copyAct, &QAction::triggered, this, &MainWindow::copyapl);
  fileMenu->addAction(copyAct);
#endif

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

  QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
  QAction *generalAct =
    settingsMenu->addAction(tr("&General"), this, &MainWindow::setGeneral);
  generalAct->setStatusTip(tr("General settings"));

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAct =
    helpMenu->addAction(tr("&About"), this, &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction *aboutQtAct =
    helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

/*******  WARNING:  TINKER WITH THESE ONLY IF YOU'RE A MASOCHIST!!  *******/
#define CLINE_RE "(([[:alpha:]][[:alnum:]]*)\\s*)"
      // 0 = whole thing
      // 1 = cmd with trailing spaces
      // 2 = cmd

#define ARG_RE "(((-[[:alpha:]][[:alnum:]]*)\\s+\
(\"([^\"]*)\"|([[:alnum:]]*))?\\s*)|\
(\"([^\"]*)\")\\s*)"
      // 0 = whole thing
      // 1 = whole thing
      // 2 = option flag
      // 3 = option val
      // 4 = quoted val
      // 5 = unquoted val
      // 6 = +cmd

static const QRegularExpression cre (CLINE_RE,
			       QRegularExpression::CaseInsensitiveOption);
static const QRegularExpression are (ARG_RE,
			       QRegularExpression::CaseInsensitiveOption);

void
MainWindow::process_line(QString text)
{
  QString outString;
  QString errString;
  text = text.trimmed ();
  aplline->setText ("");

  if (text.startsWith (QString ("∇")) &&
      !text.endsWith (QString ("∇"))) {		// function
    bool isLambda = text.startsWith (QString ("∇∇"));
    bool isNew = true;
    text = text.remove (0, (isLambda ? 2 : 1)).trimmed ();

    QString cmd = QString ("⎕cr '%1'").arg(text);
    QStringList list;
    AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
    if (!outString.isEmpty ()) {
      list = outString.split (QChar ('\n'));
      isLambda |= list.first ().contains (QString ("λ"));
      isNew = false;
    }
    QString fn;
    if (isLambda) {
      if (isNew || ((list.size () == 3) && (list[2].isEmpty ()))) 
	fn = QString ("%1/%2%3.apl").
	  arg (tempdir.path ()).arg (LAMBDA_HEADER).arg (text);
      else if ((list.size () != 0) && (list.size () != 2)){
	// fixme invalid lambda
	return;
      }
    }
    else
	fn = QString ("%1/%2.apl").arg (tempdir.path ()).arg (text);
    QFile file (fn);
    if  (file.open (QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      if (outString.isEmpty ()) {
	if (!isLambda)
	  out << text;			// boilerplate fcn header
      }
      else {
	if (isLambda && !list.isEmpty ())
	  out << list[1];
	else if (!outString.isEmpty ())
	  out << outString;
      }	
      file.close ();
      
      QStringList args;
      QString real_ed;
      {
	QString editor_copy = editor;
	QRegularExpressionMatch match = cre.match (editor_copy);
	if (match.hasMatch ()) {
	  QStringList matches = match.capturedTexts ();
	  int offset = match.capturedLength (0);
	  editor_copy.remove (0, offset);
	  real_ed = matches[2];
	  while (!editor_copy.isEmpty ()) {
	    match = are.match (editor_copy);
	    if (match.hasMatch ()) {
	      matches = match.capturedTexts ();
	      offset = match.capturedLength (0);
	      editor_copy.remove (0, offset);
	      if (!matches[2].isEmpty ()) {	// -opt version
		args << matches[3];
		args << ((matches[5].isEmpty ()) ? matches[6] : matches[5]);
	      }
	      else {				// +opt version
		args << matches[8];
	      }
	    }
	  }
	}
      }
      args << fn;

      QProcess *edit = new QProcess ();
#if 1
      connect (edit,
	       &QProcess::errorOccurred,
	       this,
	       &MainWindow::gvimErr);
      connect (edit,
	       QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
	       this,
	       &MainWindow::gvimDone);
#endif
      watcher.addPath (fn);
#if 0
      fprintf (stderr, "ed \"%s\"\n", toCString (real_ed));
      int j;
      for (j = 0; j < args.size (); j++)
	fprintf (stderr, "a[%d] \"%s\"\n", j, toCString (args[j]));
#endif
      edit->start (real_ed, args);
    }
    else {
      // fixme file open error
    }
    return;
  }
  
  aplwin->append (text);

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
  aplwin->moveCursor (QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
  aplwin->ensureCursorVisible();
}

void MainWindow::returnPressed()
{
  QString text = aplline->text();
  history->insert (toCString (text));
  process_line (text);
  history->rebase ();
}

static void
do_page (MainWindow  *mainwin, int dir)
{
  QFont font = mainwin->aplwin->currentFont ();
  // the 12 and 20 are hacks to get the line spacing
  int lines =  (12 * mainwin->aplwin->height ()) /
    (20 * font.pointSize ());
  lines = (lines > 8) ? lines -2 : 1;
  int i;
  if (dir > 0) {
    for (i = 0; i < lines; i++) 
      mainwin->aplwin->moveCursor (QTextCursor::Up,
				   QTextCursor::MoveAnchor);
  }
  else {
    for (i = 0; i < lines; i++) 
      mainwin->aplwin->moveCursor (QTextCursor::Down,
				   QTextCursor::MoveAnchor);
  }
}

bool
AplWinFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == watched) {
    if (event->type() == QEvent::Wheel) {
      QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
      // cw -120, ccw 120
      int mv = (int)wheelEvent->delta ();
      mainwin->aplwin->moveCursor (((mv > 0) ? QTextCursor::Up
				    : QTextCursor::Down),
				   QTextCursor::MoveAnchor);
      return true;
    }
    else if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_PageUp) {
	do_page (mainwin, 1);
	return true;
      }
      else if (keyEvent->key() == Qt::Key_PageDown) {
	do_page (mainwin, -1);
	return true;
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

#define APL_VARIABLE "([∆a-z][∆_a-z0-9]*)"

static void
lineKey (MainWindow  *mainwin, int dir)
{
  char *str = (dir > 0)
    ? mainwin->history->previous () 
    : mainwin->history->next ();
  QString text = QString (str);
  mainwin->aplline->setText (text);
}

bool
AplLineFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == watched) {
    if (event->type() == QEvent::Wheel) {
      QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
      lineKey (mainwin, (int)wheelEvent->delta ());
    }
    else if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_PageUp) {
	do_page (mainwin, 1);
	return true;
      }
      else if (keyEvent->key() == Qt::Key_PageDown) {
	do_page (mainwin, -1);
	return true;
      }
      else if (keyEvent->key() == Qt::Key_Up) {
	lineKey (mainwin, 1);
	return true;
      }
      else if (keyEvent->key() == Qt::Key_Down) {
	lineKey (mainwin, -1);
	return true;
      }
      else if (keyEvent->key() == Qt::Key_Tab) {
	QRegularExpression aplsep (APL_VARIABLE);
	aplsep.setPatternOptions (QRegularExpression::CaseInsensitiveOption);
	QString outString;
	QString errString;
	QString cmd = QString (")fns");
	AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
	QString vbls = outString;
	cmd = QString (")vars");
	AplExec::aplExec (APL_OP_COMMAND, cmd, outString, errString);
	vbls.append (outString);
	if (!vbls.isEmpty ()) {
	  QString text = mainwin->aplline->text();
	  QRegularExpressionMatchIterator toks = aplsep.globalMatch (text);
	  QString tok;
	  while (toks.hasNext ()) {
	    QRegularExpressionMatch match = toks.next(); 
	    tok = match.captured(1);
	  }
	  if (!tok.isEmpty ()) {
	    int i;
	    QStringList fns = vbls.split (QRegExp ("\\s+"));
	    QStringList possibles = QStringList ();
	    for (i = 0; i < fns.size (); i++)
	      if (fns[i].startsWith (tok)) possibles.push_back (fns[i]);
	    if (0 < possibles.size ()) {
	      if (1 == possibles.size ()) {
		text.chop (tok.size ());
		text.append (possibles[0]);
		mainwin->aplline->setText (text);
	      }
	      else {
		QDialog dialog (mainwin, Qt::Popup);
		QVBoxLayout *layout = new QVBoxLayout ();
		dialog.setLayout (layout);
		QComboBox *completion_ops =  new QComboBox ();
		completion_ops->setMaxVisibleItems (10);
		completion_ops->showPopup ();
		for (i = 0; i < possibles.size (); i++)
		  completion_ops->addItem (possibles[i]);
		layout->addWidget(completion_ops);
		dialog.exec ();
		QString sel = completion_ops->currentText ();
		text.chop (tok.size ());
		text.append (sel);
		mainwin->aplline->setText (text);
		delete completion_ops;
		delete layout;
	      }
	    }
	  }
	}
	else if (!errString.isEmpty ()) {
	  fprintf (stderr, "got fns err\n");
	}
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
    aplWinFilter = new AplWinFilter (aplwin, this);
    aplwin->setReadOnly (true);
    aplwin->setEnabled (true);
    aplwin->installEventFilter(aplWinFilter);
    if (!msgs.isEmpty ()) aplwin->setText (msgs);
    layout->addWidget (aplwin);

    aplline = new  QLineEdit ();
    aplline->setPlaceholderText ("APL");
    aplLineFilter = new AplLineFilter (aplline, this);
    aplline->setEnabled (true);
    aplline->installEventFilter(aplLineFilter);
    connect(aplline, &QLineEdit::returnPressed,
	    this, &MainWindow::returnPressed);
    layout->addWidget (aplline);

    formGroupBox->setLayout (layout);
    formGroupBox->setAlignment (Qt::AlignLeft);
    outerlayout->addWidget (formGroupBox);
  }
  {
    QGroupBox *formGroupBox = new QGroupBox (QString ("Chart control"));
    QGridLayout *layout = new QGridLayout;
    QMenuBar *mb2 = new QMenuBar ();
    QMenu *fileMenu = mb2->addMenu(tr("&File"));
    QAction *openAct = new QAction(tr("&Open Chart..."), this);
    openAct->setStatusTip(tr("Open an existing vis file"));
    //    connect(openAct, &QAction::triggered, this, &MainWindow::open);
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
  QSettings settings;
  editor = settings.value (SETTINGS_EDITOR).toString ();
  if (editor.isEmpty ()) 
    editor = QString (DEFAULT_EDITOR);
  save_mode = SAVE_MODE_SAVE;
  connect(&watcher,
	  &QFileSystemWatcher::fileChanged,
	  this, &MainWindow::fileChanged);
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
  
  aplline->setFocus ();
  this->show ();

  changed = false;
}

MainWindow::~MainWindow()
{
}

