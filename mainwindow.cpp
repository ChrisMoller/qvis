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
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QPolarChart>
#include <QMenuBar>
#include <complex>
#include <values.h>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"
#include "xml.h"

#define expvar "expvarλ"

//static QHash<const QString, int> xmlhash;

void
MainWindow::closeEvent(QCloseEvent *event __attribute__((unused)))
{
  // chart x-ed out first
  fprintf (stderr, "closeEvent\n");
  byebye ();
}

void
MainWindow::themeChanged (int newtheme __attribute__((unused)))
{
  theme = (QChart::ChartTheme)themebox->currentData ().toInt ();
  handleExpression ();
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
    settings.setValue (HEIGHT, chartView->height ());
    settings.setValue (WIDTH, chartView->width ());
    QCoreApplication::quit ();
  }
}

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

int
MainWindow::handle_vector (APL_value res,
			   APL_value xvals,
			   QString flbl)
{
  uint64_t count = get_element_count (res);

  int res_type = -1;
  std::vector<std::complex<double>> vect (count);
  for (uint64_t c = 0; c < count; c++) {
    if (is_numeric (res, c)) {
      if (res_type == -1) res_type = CCT_NUMERIC;
      if (is_complex (res, c)) {
	if (res_type == CCT_NUMERIC) res_type = CCT_COMPLEX;
	vect[c] = std::complex<double> ((double)get_real (res, c),
					(double)get_imag (res, c));
      }
      else
	vect[c] = std::complex<double> ((double)get_real (res, c), 0.0);
    }
  }
  
  int frc = 0;
  
  chartView->chart ()->removeAllSeries();
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;
  //  qreal z_max = -MAXDOUBLE;
  //  qreal z_min =  MAXDOUBLE;

  if (res_type == CCT_COMPLEX) {
    // fixme
    // complex vector vs idx, rank = 1
  }
  else {
    // real vector vs idx, rank = 1
    QSplineSeries *sseries = nullptr;
    QLineSeries   *pseries = nullptr;

    settings.setValue (DO_SPLINE,  curve.spline);
    if (curve.spline) {
      sseries = new QSplineSeries ();
      sseries->setName(flbl);
    }
    else {
      pseries = new QLineSeries ();
      pseries->setName(flbl);
    }

    for (uint64_t i = 0; i < count; i++) {
      qreal y_val = (qreal)vect[i].real ();
      if (y_max < y_val) y_max = y_val;
      if (y_min > y_val) y_min = y_val;
      if (sseries) sseries->append ((qreal)get_real (xvals, i), y_val);
      else pseries->append ((qreal)get_real (xvals, i), y_val);
    }

    if (sseries)
      chartView->chart ()->addSeries (sseries);
    else
      chartView->chart ()->addSeries (pseries);

    chartView->chart ()->createDefaultAxes ();
    
    qreal dy = 0.075 * (y_max - y_min);
    chartView->chart ()->axes (Qt::Vertical).first()
      ->setRange(y_min-dy, y_max+dy);
    frc = 1;
  }

  return frc;
}

void
MainWindow::handleExpression ()
{
  settings.setValue (DO_POLAR, curve.polar);
  chartView->setChart (curve.polar ? polarchart : chart);

  int incr = 16;  // fixme--make settable

  if (!curve.ix.name.isEmpty ()) {
    /***
	lbl ← min + ((⍳incr+1)-⎕io) × (max - min) ÷ incr
    ***/
    QString range_x =
      QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
      .arg(curve.ix.name).arg(curve.ix.range.min).arg(incr)
      .arg(curve.ix.range.max);
    apl_exec (range_x.toStdString ().c_str ());
    
    char loc[256];
    sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
    APL_value xvals =
      get_var_value (curve.ix.name.toStdString ().c_str (), loc);

    //    https://doc.qt.io/qt-5/qtcharts-splinechart-example.html

    bool zset = false;
    if (!curve.iz.name.isEmpty ()) {
      settings.setValue (Z_VAR_NAME, curve.iz.name);
      settings.setValue (Z_VAR_MIN,  curve.iz.range.min);
      settings.setValue (Z_VAR_MAX,  curve.iz.range.max);
      QString range_z =
	QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
	.arg(curve.iz.name).arg(curve.iz.range.min)
	.arg(incr).arg(curve.iz.range.max);
      zset = true;
      apl_exec (range_z.toStdString ().c_str ());
    }

    QString input = apl_expression->text ();
    if (input.isEmpty ()) return;
    QString fcn = QString ("%1  ← %2").arg (expvar).arg (input);
    int xrc = apl_exec (fcn.toStdString ().c_str ());
    if (xrc != LAE_NO_ERROR) return;

    sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
    APL_value res = get_var_value (expvar, loc);

    if (res) {
      int frc = 0;

      int rank = get_rank (res);

      // curves

      switch (rank)  {
      case 1:
	// fixme -- don't need to  pass lbl
	frc =  handle_vector (res, xvals, curve.function.title);
	break;
      case 2:
	// real array vs idx, rank = 2, one curve per row

	// real parametric array, rank = 2, row !sel vs row sel,
	//       one curve per !sel

      
	// complex array vs idx, rank = 2, one curve per row

	// complex parametric array, rank = 2, row !sel vs row sel, sel real
      
	// real parametric array, rank > 2, row !sel vs row sel,
	//       one curve per !sel, sel real

	// surfaces

	// real array vs ix, iz, rank = 2


	// complex array rank = 2, !sel vs re(sel),im(sel)

	  break;
      case 3:
	// real array vs ix, iz, rank = 3. one surface plane
	break;

      default:
	break;
      }

	

      if (frc) {
	settings.setValue (X_VAR_NAME, curve.ix.name);
	settings.setValue (X_VAR_MIN,  curve.ix.range.min);
	settings.setValue (X_VAR_MAX,  curve.ix.range.max);
	settings.setValue (FCN_LABEL,  curve.function.title);
	settings.setValue (FUNCTION,   input);

	settings.setValue (HEIGHT, chartView->height ());
	settings.setValue (WIDTH, chartView->width ());

	settings.setValue (THEME,  theme);
	chartView->chart ()->setTheme (theme);

	settings.setValue (CHART_TITLE,  curve.title);
	chartView->chart ()->setTitle (curve.title);
      
	//	QString x_ttl = x_title->text ();
	settings.setValue (X_TITLE,  curve.ix.title);
	chartView->chart ()->axes (Qt::Horizontal).first()
	  ->setTitleText(curve.ix.title);

	settings.setValue (Y_TITLE,  curve.function.label);
	chartView->chart ()->axes (Qt::Vertical).first()
	  ->setTitleText(curve.function.label);

	QString cmd =
	  QString (")erase %1 %2").arg (expvar).arg (curve.ix.name);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (xvals, loc);
	apl_command (cmd.toStdString ().c_str ());
	if (zset) {
	  cmd = QString (")erase %1").arg (curve.iz.name);
	  apl_command (cmd.toStdString ().c_str ());
	  // fixme
	  // sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	  // release_value (zvals, loc);
	}
      }
    }
  }
}

void
MainWindow::valChanged (bool enabled __attribute__((unused)))
{
  curve.polar  = (Qt::Checked == do_polar->checkState());
  curve.spline = (Qt::Checked == do_spline->checkState());
  //  curve.shorttitle
  curve.title			= chart_title->text ();
  curve.function.label		= y_title->text ();
  curve.function.title		= fcn_label->text ();
  curve.function.expression	= apl_expression->text ();
  curve.ix.name			= x_var_name->text ();
  curve.ix.title		= x_title->text ();
  curve.ix.range.min		= x_var_min->value ();
  curve.ix.range.max		= x_var_max->value ();
  curve.iz.name			= z_var_name->text ();
  curve.iz.title		= z_title->text ();
  curve.iz.range.min 		= z_var_min->value ();
  curve.iz.range.max 		= z_var_max->value ();
  changed = true;
  handleExpression ();
}

void
MainWindow::valChangedv ()
{
  valChanged (true);
}

void
MainWindow::newFile()
{
#if 0
  QChartView *newChartView = new QChartView ();
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
    readFile(dialog.selectedFiles().first());
}

bool
MainWindow::save()
{
  if (curFile.isEmpty()) {
    return saveAs();
  } else {
    return saveFile(curFile);
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
  return saveFile(dialog.selectedFiles().first());
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

  const QIcon exitIcon = QIcon::fromTheme("application-exit");
  QAction *exitAct =
    fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));

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

void
MainWindow::buildMenu (MainWindow *win, QChart *chart,
			QPolarChart *polarchart)
{
  create_menuBar ();
  QGroupBox *formGroupBox = new QGroupBox ("Visualisation");
  QGridLayout *layout = new QGridLayout;

  QSettings settings;

  int row = 0;
  int col = 0;

  curve.title = settings.value (CHART_TITLE).toString ();
  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  chart_title->setText (curve.title);
  layout->addWidget (chart_title, row, 0, 1, 3);
  
  curve.function.label = settings.value (Y_TITLE).toString ();
  y_title = new  QLineEdit ();
  y_title->setPlaceholderText ("y axix label");
  y_title->setText (curve.function.label);
  layout->addWidget (y_title, row, 3);
  
  /*  x indep vbl */

  row++;
  col = 0;
  
  curve.ix.name = settings.value (X_VAR_NAME).toString ();
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  x_var_name->setText (curve.ix.name);
  layout->addWidget (x_var_name, row, col++);
  
  curve.ix.range.min = settings.value (X_VAR_MIN).toDouble ();
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_min->setToolTip ("x minimum value");
  x_var_min->setValue (curve.ix.range.min);
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_min, row, col++);

  curve.ix.range.max = settings.value (X_VAR_MAX).toDouble ();
  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_max->setToolTip ("x maximum value");
  x_var_max->setValue (curve.ix.range.max);
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (x_var_max, row, col++);
  
  curve.ix.title = settings.value (X_TITLE).toString ();
  x_title = new  QLineEdit ();
  x_title->setPlaceholderText ("x axix label");
  x_title->setText (curve.ix.title);
  layout->addWidget (x_title, row, col++);

  /*  z indep vbl */

  row++;
  col = 0;

  curve.iz.name = settings.value (Z_VAR_NAME).toString ();
  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  z_var_name->setText (curve.iz.name);
  layout->addWidget (z_var_name, row, col++);

  curve.iz.range.min = settings.value (Z_VAR_MIN).toDouble ();
  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_min->setToolTip ("z minimum value");
  z_var_min->setValue (curve.iz.range.min);
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_min, row, col++);

  curve.iz.range.max = settings.value (Z_VAR_MAX).toDouble ();
  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_max->setToolTip ("z maximum value");
  z_var_max->setValue (curve.iz.range.max);
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &MainWindow::valChanged);
  layout->addWidget (z_var_max, row, col++);
  
  curve.iz.title = settings.value (Z_TITLE).toString ();
  z_title = new  QLineEdit ();
  z_title->setPlaceholderText ("z axix label");
  z_title->setText (curve.iz.title);
  layout->addWidget (z_title, row, col++);


  /*  APL expression */

  row++;
  col = 0;
  curve.function.title = settings.value (FCN_LABEL).toString ();
  fcn_label = new  QLineEdit ();
  fcn_label->setPlaceholderText ("curve label");
  fcn_label->setText (curve.function.title);
  layout->addWidget (fcn_label, row, col++);

  curve.function.expression = settings.value (FUNCTION).toString ();
  apl_expression = new  QLineEdit ();
  apl_expression->setPlaceholderText ("function");
  apl_expression->setText (curve.function.expression);
  layout->addWidget (apl_expression, row, col, 1, 3);
  QObject::connect (apl_expression,
		    &QLineEdit::editingFinished,
		    this,
		    &MainWindow::valChangedv);

  /*  toggles */
  
  row++;
  col = 0;
  
  curve.spline = settings.value (DO_SPLINE).toBool ();
  do_spline = new QCheckBox ("Spline");
  do_spline->setCheckState (curve.spline ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);
  
  curve.polar = settings.value (DO_POLAR).toBool ();
  do_polar = new QCheckBox ("Polar");
  do_polar->setCheckState (curve.polar ? Qt::Checked : Qt::Unchecked);
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  &MainWindow::valChanged);


  /*   buttons */

  /*   settings button   */

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



  /*   end buttons  */

  formGroupBox->setLayout (layout);
  formGroupBox->show ();

  chartView->setChart (curve.polar ? polarchart : chart);
}

MainWindow::MainWindow (QWidget *parent)
  : QMainWindow(parent)
{
  initXmlHash ();
  
  chart      = new QChart ();
  polarchart = new QPolarChart ();
  chartView = new QChartView ();
  chartView->setRenderHint (QPainter::Antialiasing);

  this->setCentralWidget (chartView);
  
  QVariant ww = settings.value (WIDTH);
  QVariant hh = settings.value (HEIGHT);
  if (ww.isValid () && hh.isValid ()) 
    this->resize (ww.toInt (), hh.toInt ());
  
  QVariant tt = settings.value (THEME);
  theme = tt.isValid ()
    ? (QChart::ChartTheme)tt.toInt ()
    :  QChart::ChartThemeBlueCerulean;

  buildMenu (this, chart, polarchart);

  this->show ();

  curves.push_back (curve);
  handleExpression ();
  changed = false;
}

MainWindow::~MainWindow()
{

}

