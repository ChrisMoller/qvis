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

QAbstractSeries *
ChartWindow::handle_vector (qreal &y_max,
			    qreal &y_min,
			    APL_value res,
			    QVector<double> &xvals,
			    bool spline,
			    Curve *curve)
{
  QString flbl      = curve->getName ();
  QColor  fcolour   = curve->getColour ();
  Qt::PenStyle fpen = curve->getPen ();
  uint64_t count    = get_element_count (res);
  QAbstractSeries *series = nullptr;

  int res_type = -1;
  std::vector<std::complex<double>> vect (count);  // fixme use qvector
  bool has_cpx = false;
  for (uint64_t c = 0; c < count; c++) {
    if (is_numeric (res, c)) {
      if (res_type == -1) res_type = CCT_NUMERIC;
      if (is_complex (res, c)) {
	if (res_type == CCT_NUMERIC) res_type = CCT_COMPLEX;
	vect[c] = std::complex<double> ((double)get_real (res, c),
					(double)get_imag (res, c));
	has_cpx = true;
      }
      else
	vect[c] = std::complex<double> ((double)get_real (res, c), 0.0);
    }
  }

  if (has_cpx) {
    for (uint64_t c = 0; c < count; c++) {
      switch (curve->getCpx ()) {
      case CPX_REAL:
	break;
      case CPX_IMAG:
	vect[c].real (vect[c].imag ());
	break;
      case CPX_MAG:
	vect[c].real (abs (vect[c]));
	break;
      case CPX_PHASE:
	vect[c].real (arg (vect[c]));
	break;
      }
    }
  }
  
  {
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
      series = sseries;
    }
    else {
      pseries = new QLineSeries ();
      pseries->setName(flbl);
      pseries->setColor (fcolour);
      pseries->setPen (pen);
      pseries->setPointsVisible (curve->getPointsVisible ());
      pseries->setPointLabelsVisible (curve->getPointLabelsVisible ());
      series = pseries;
    }
    
    int i;
    for (i = 0; i < (int)count; i++) {
      qreal y_val = (qreal)vect[i].real ();
      if (y_max < y_val) y_max = y_val;
      if (y_min > y_val) y_min = y_val;
      if (sseries) sseries->append ((qreal)xvals[i], y_val);
      else pseries->append ((qreal)xvals[i], y_val);
    }
  }

  return series;
}

void
ChartWindow::eraseIndex (Index *idx)
{
  QString outString;
  QString errString;
  QString name = idx->getName ();
  if (!name.isEmpty ()) {
    QString cmd =
      QString (")erase %1").arg (name);
    AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
  }
}

QVector<double> 
ChartWindow::setIndex (Index *idx, int incr, QString title)
{
  QVector<double>vals;
  QString name = idx->getName ();
  if (!name.isEmpty ()) {
    int i;
    double  min  = idx->getMin ();
    double  max  = idx->getMax ();
    QByteArray nameUtf8 = name.toUtf8();
    char loc[256];
    sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
    APL_value res = apl_vector ((int64_t)(incr +1), loc);

    for (i = 0; i <= incr; i++) {
      double val = min + ((double)i/(double)incr) * (max - min);
      vals.append (val);
      set_double ((APL_Float)val, res, (uint64_t)i);
    }
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
  MainWindow *mw = chartControls->getMainWindow ();
  QString outString;
  QString errString;
  bool polar  = (Qt::Checked == chartControls->do_polar->checkState ());
  bool spline = (Qt::Checked == chartControls->do_spline->checkState ());

  chartView->setChart (polar ? polarchart : chart);

  ChartData *cd = chartControls->getChartData ();
  chartView->chart ()->setTheme (cd->getTheme ());
  if (!polar) {
    QString fn = chartControls->getChartData ()->getBGFile ();
    if (!fn.isEmpty ()) {
      QImage gep (fn);
      chartView->setRenderHint(QPainter::Antialiasing, true);
      int width = static_cast<int>(chart->plotArea().width());
      int height = static_cast<int>(chart->plotArea().height());
      int ViewW = static_cast<int>(chartView->width());
      int ViewH = static_cast<int>(chartView->height());

      gep = gep.scaled(QSize(width, height));

      QImage translated(ViewW, ViewH, QImage::Format_ARGB32);
      translated.fill(Qt::white);
      QPainter painter(&translated);
      QPointF TopLeft = chart->plotArea().topLeft();
      painter.drawImage(TopLeft, gep);

      chart->setPlotAreaBackgroundBrush(translated);
      chart->setPlotAreaBackgroundVisible(true);
    }
    else 
      chart->setPlotAreaBackgroundVisible(false);
  }
  
  chartView->chart ()->setTitle (chartControls->chart_title->text ());

  Index *ix = chartControls->getChartData ()->getXIndex ();
  QVector<double> xvals =
    setIndex (ix, incr, chartControls->chart_title->text ());
  
  Index *iz = chartControls->getChartData ()->getZIndex ();
  QVector<double> zvals =
    setIndex (iz, incr, chartControls->chart_title->text ());
  
  QList<int> sels =  chartControls->getChartData ()->getSelected ();
  bool chart_created = false;

  QString curve_label;
  QList<QAbstractSeries *>series_list;
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;
  
	
  for (i =  0; i < sels.size (); i++) {
    Curve curve = mw->getCurve (sels[i]);
    curve_label = curve.getLabel ();		// set to last curve
    QString fcn = curve.getFunction ();
    QString stmt = QString ("%1 ← %2").arg (expvar).arg (fcn);
      
    outString.clear ();
    errString.clear ();
    AplExec::aplExec (APL_OP_EXEC, stmt, outString, errString);
    mw->update_screen (errString, outString);

    if (errString.isEmpty ()) {
      sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
      APL_value res = get_var_value (expvar, loc);
      if (res) {
	QAbstractSeries *frc =
	  handle_vector (y_max, y_min, res, xvals, spline, &curve);
	series_list.append (frc);
	QString cmd =
	  QString (")erase %1").arg (expvar);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
      }
    }
  }
  
  chartView->chart ()->removeAllSeries();
  for (i = 0; i < series_list.size (); i++) {
    if (series_list[i]) chartView->chart ()->addSeries (series_list[i]);
    chart_created = true;
  }

  if (chart_created) {
    chartView->chart ()->createDefaultAxes ();
    
    qreal dy = 0.075 * (y_max - y_min);
    chartView->chart ()->axes (Qt::Vertical).first()
      ->setRange(y_min-dy, y_max+dy);  
  
    QString ix_label = ix->getLabel ();
    chartView->chart ()->axes (Qt::Horizontal).first()
      ->setTitleText (ix_label);
    chartView->chart ()->axes (Qt::Vertical).first()
      ->setTitleText (curve_label);
  }

  eraseIndex (ix);
  eraseIndex (iz);
}

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
  chartControls->getMainWindow ()->eraseParams ();

#if 0
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
