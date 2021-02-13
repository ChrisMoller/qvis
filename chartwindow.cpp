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
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QPolarChart>
#include <QMenuBar>
#include <complex>
#include <values.h>
#include <iconv.h>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

class ChartWindow;
#include "mainwindow.h"
#include "chartcontrols.h"
#include "chartwindow.h"
#include "aplexec.h"


#define expvar "expvarλ"

int
ChartWindow::handle_vector (APL_value res,
			    QVector<double> &xvals,
			    bool spline, Curve *curve)
{
  QString flbl      = curve->getName ();
  QColor  fcolour   = curve->getColour ();
  Qt::PenStyle fpen = curve->getPen ();
  uint64_t count    = get_element_count (res);

  int res_type = -1;
  std::vector<std::complex<double>> vect (count);  // fixme use qvector
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
    QPen pen (fcolour);
    pen.setStyle (fpen);

    if (spline) {
      sseries = new QSplineSeries ();
      sseries->setName (flbl);
      sseries->setColor (fcolour);
      sseries->setPen (pen);
    }
    else {
      pseries = new QLineSeries ();
      pseries->setName(flbl);
      pseries->setColor (fcolour);
      pseries->setPen (pen);
    }

    int i;
    for (i = 0; i < (int)count; i++) {
      qreal y_val = (qreal)vect[i].real ();
      if (y_max < y_val) y_max = y_val;
      if (y_min > y_val) y_min = y_val;
      if (sseries) sseries->append ((qreal)xvals[i], y_val);
      else pseries->append ((qreal)xvals[i], y_val);
    }

    if (sseries) chartView->chart ()->addSeries (sseries);
    else         chartView->chart ()->addSeries (pseries);

    chartView->chart ()->createDefaultAxes ();
    
    qreal dy = 0.075 * (y_max - y_min);
    chartView->chart ()->axes (Qt::Vertical).first()
      ->setRange(y_min-dy, y_max+dy);
    frc = 1;
  }

  return frc;
}

QVector<double> 
ChartWindow::setIndex (Index *idx, int incr, QString title)
{
  QVector<double>vals;
  QString name = idx->getName ();
  if (!name.isEmpty ()) {
    double  min  = idx->getMin ();
    double  max  = idx->getMax ();
    char loc[256];
    sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
    APL_value res = apl_vector ((int64_t)(incr +1), loc);

    int i;
    for (i = 0; i <= incr; i++) {
      double val = min + ((double)i/(double)incr) * (max - min);
      vals.append (val);
      set_double ((APL_Float)val, res, (uint64_t)i);
    }
    QByteArray nameUtf8 = name.toUtf8();
    sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
    int src = set_var_value (nameUtf8.constData (), res, loc);
    if (src != 0) {
      QMessageBox msgBox;
      QString msg =
	QString ("Error setting index %1 in chart %2").arg (name).arg (title);
      msgBox.setText (msg);
      msgBox.setIcon (QMessageBox::Warning);
      msgBox.exec();
    }
    release_value (res, loc);
  }
  return vals;
}

void
ChartWindow::drawChart ()
{
  int i;
  char loc[256];
  int incr = 16;  // fixme--make settable
  QString outString;
  QString errString;
  bool polar  = (Qt::Checked == chartControls->do_polar->checkState ());
  bool spline = (Qt::Checked == chartControls->do_spline->checkState ());

  chartView->setChart (polar ? polarchart : chart);
  chartView->chart ()->setTheme (chartControls->getChartData ()->getTheme ());
  chartView->chart ()->setTitle (chartControls->chart_title->text ());

  MainWindow *mw = chartControls->getMainWindow ();
  QList<Param> params = mw->getParams ();

  Index *ix = chartControls->getChartData ()->getXIndex ();
  QVector<double> xvals =
    setIndex (ix, incr, chartControls->chart_title->text ());
  
  Index *iz = chartControls->getChartData ()->getZIndex ();
  QVector<double> zvals =
    setIndex (iz, incr, chartControls->chart_title->text ());
  
  //  QString iz_label = iz->getLabel ();

  sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
  QList<int> sels =  chartControls->getChartData ()->getSelected ();
  bool chart_created = false;

  QString curve_label;
  for (i =  0; i < sels.size (); i++) {
    Curve curve = mw->getCurve (sels[i]);
    curve_label = curve.getLabel ();		// set to last curve
    QString fcn = curve.getFunction ();
    QString stmt = QString ("%1←%2").arg (expvar).arg (fcn);
    AplExec::aplExec (APL_OP_EXEC, stmt, outString, errString);
    mw->update_screen (errString, outString);
    
    if (errString.isEmpty ()) {
      sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
      APL_value res = get_var_value (expvar, loc);
      if (res) {
	handle_vector (res, xvals, spline, &curve);
	QString cmd =
	  QString (")erase %1").arg (expvar);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
	chart_created = true;
      }
    }
  }
  if (chart_created) {
    QString ix_label = ix->getLabel ();
    chartView->chart ()->axes (Qt::Horizontal).first()
      ->setTitleText (ix_label);
    chartView->chart ()->axes (Qt::Vertical).first()
      ->setTitleText (curve_label);
  }

#if 0
  for (i = 0; i < mw->getCurveCount (); i++) {
    Curve curve = mw->getCurve (i);
    QString fcn = curve.getFunction ();
    QString stmt = QString ("%1  ← %2").arg (expvar).arg (fcn);
    fprintf (stderr, "execing \"%s\"\n", toCString (stmt));
    AplExec::aplExec (APL_OP_EXEC, stmt, outString, errString);
    mw->update_screen (errString, outString);

#if 0
    if (!errString.isEmpty ()) {
      sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
      APL_value res = get_var_value (expvar, loc);
      if (res) {
	//	int frc =  handle_vector (res, xvals, "mmmm");
	QString cmd =
	  QString (")erase %1").arg (expvar);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
      }
    }
#endif
  }
#endif
}

#if 0
void
ChartWindow::handleExpression ()
{
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
      QString range_z =
	QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
	.arg(curve.iz.name).arg(curve.iz.range.min)
	.arg(incr).arg(curve.iz.range.max);
      zset = true;
      apl_exec (range_z.toStdString ().c_str ());
    }

    QString input = curve.function.expression;

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
	chartView->chart ()->setTheme (theme);	// fixme
	chart->setTitleFont (titlefont);	// fixme
	chart->legend ()->setFont (titlefont);
	chartView->chart ()->setTitle (curve.title);
      
	chartView->chart ()->axes (Qt::Horizontal).first()
	  ->setTitleText(curve.ix.title);
	chartView->chart ()->axes (Qt::Horizontal).first()
	  ->setTitleFont(titlefont);

	chartView->chart ()->axes (Qt::Vertical).first()
	  ->setTitleText(curve.function.label);
	chartView->chart ()->axes (Qt::Vertical).first()
	  ->setTitleFont(titlefont);

	QString cmd =
	  QString (")erase %1 %2").arg (expvar).arg (curve.ix.name);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (xvals, loc);
	const char *res = apl_command (cmd.toStdString ().c_str ());
	if (res) free (res); 
	if (zset) {
	  cmd = QString (")erase %1").arg (curve.iz.name);
	  res = apl_command (cmd.toStdString ().c_str ());
	  if (res) free (res);
	  // fixme
	  // sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	  // release_value (zvals, loc);
	}
      }
    }
  }
}
#endif

void
ChartWindow::imageExport()
{
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"));
  dialog.setViewMode(QFileDialog::Detail);
  QStringList fileNames;
  if (dialog.exec()) {
    fileNames = dialog.selectedFiles();
    QString fn = fileNames.first ();
    QPixmap p = chartView->grab();
    p.save(fn);
  }
}

#if 0
void
ChartWindow::themeChanged (int newtheme __attribute__((unused)))
{
  theme = (QChart::ChartTheme)themebox->currentData ().toInt ();
  handleExpression ();
}
#endif

#if 0
void
ChartWindow::settheme()
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

  themebox->setCurrentIndex (theme);
  
  connect(themebox,
	  QOverload<int>::of(&QComboBox::activated),
	  this,
	  &ChartWindow::themeChanged);
  
  layout->addWidget(themebox, 0, 0);
  
  QDialog dialog;
  dialog.setLayout (layout);
  dialog.exec ();
  changed = true;
}
#endif

#if 0
void
ChartWindow::setfont()
{
  QFontDialog dialog (chart->titleFont ());
  bool ok;
  QFont newfont = QFontDialog::getFont(&ok, chart->font (), this);
  if (ok) {
    titlefont = newfont;
    handleExpression ();
  }
}

void
ChartWindow::create_menuBar ()
{
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

  QToolBar *fileToolBar = addToolBar(tr("File"));
  const QIcon imageIcon =
    QIcon::fromTheme("camera-photo", QIcon(":/images/camera-photo.png"));
  QAction *imageAct = new QAction(imageIcon, tr("&Export image"), this);
  //  newAct->setShortcuts(QKeySequence::New);
  imageAct->setStatusTip(tr("Export an image"));
  connect(imageAct, &QAction::triggered, this, &ChartWindow::imageExport);
  fileMenu->addAction(imageAct);
  fileToolBar->addAction(imageAct);

  QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));

#if 0
  QAction *themeAct =
    settingsMenu->addAction(tr("&Theme"), this, &ChartWindow::settheme);
  themeAct->setStatusTip(tr("Set theme"));
#endif

  QAction *fontAct =
    settingsMenu->addAction(tr("&Font"), this, &ChartWindow::setfont);
  fontAct->setStatusTip(tr("Set font"));
}
#endif

ChartWindow::ChartWindow  (ChartControls *parent)
  : QMainWindow(parent)
{
  chartControls = parent;

  QVariant ww = settings.value (SETTINGS_WIDTH);
  QVariant hh = settings.value (SETTINGS_HEIGHT);
  if (ww.isValid () && hh.isValid ()) 
    this->resize (ww.toInt (), hh.toInt ());

  chart      = new QChart ();
  polarchart = new QPolarChart ();
  chartView  = new QChartView ();
  chartView->setRenderHint (QPainter::Antialiasing);

  chartControls->getMainWindow ()->setParams ();
  drawChart ();

#if 0
  /******* fake ***********/

  chartView->chart ()->setTheme (QChart::ChartThemeBlueCerulean);

  chartView->setChart (chart);
  
  chartView->chart ()->setTitle ("Fake title");

  QSplineSeries* series = new QSplineSeries();
  series->append (0, 6);
  series->append (2, 4);
  series->append (5, 5);
  series->append (3, 7);
  series->append (7, 2);
  chart->addSeries(series);

  QSplineSeries* series2 = new QSplineSeries();
  series2->append (2, 4);
  series2->append (0, 6);
  series2->append (7, 2);
  series2->append (3, 7);
  series2->append (5, 5);
  series2->setColor (QColor ("red"));
  chart->addSeries(series2);

  chartView->chart ()->createDefaultAxes ();

  chartView->chart ()->axes (Qt::Horizontal).first()
    ->setTitleText (QString ("x title"));

  chartView->chart ()->axes (Qt::Vertical).first()
    ->setTitleText (QString ("y title"));

  chartView->chart ()->axes (Qt::Vertical).first()
    ->setGridLineVisible (true);

  chartView->chart ()->axes (Qt::Horizontal).first()
    ->setGridLineVisible (true);

  chartView->chart ()->axes (Qt::Vertical).first()
    ->setGridLineVisible (true);

  chartView->chart ()->axes (Qt::Horizontal).first()
    ->setLineVisible (true);

  chartView->chart ()->axes (Qt::Vertical).first()
    ->setLineVisible (true);

  chartView->chart ()->axes (Qt::Horizontal).first()
    ->setGridLineColor (QColor ("black"));

  chartView->chart ()->axes (Qt::Vertical).first()
    ->setGridLineColor (QColor ("black"));
#endif

  chartView->chart ()->setDropShadowEnabled(true);

  
  /******* end fake ***********/
  //  create_menuBar ();
  
  this->setCentralWidget (chartView);
  this->show ();

  //curves.push_back (curve);
}

ChartWindow::~ChartWindow()
{

}
