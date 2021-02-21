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
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QPolarChart>
#include <QMenuBar>
#include <complex>
#include <values.h>
#include <iconv.h>

#include <GL/gl.h>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE
using namespace QtDataVisualization;


class ChartWindow;
#include "mainwindow.h"
#include "chartcontrols.h"
#include "chartwindow.h"
#include "aplexec.h"

QSurfaceDataArray *
ChartWindow::handle_surface (qreal &x_max, qreal &x_min,
			     qreal &y_max, qreal &y_min,
			     qreal &z_max, qreal &z_min,
			     APL_value res,
			     QVector<double> &xvals,
			     QVector<double> &zvals,
			     Curve *curve)
{
  if (get_rank (res) != 2) {
    // fixme msg popup
    return nullptr;
  }

  int r, c, p;
  int rows = get_axis (res, 0);
  int cols = get_axis (res, 1);

  QSurfaceDataArray *dataArray = new QSurfaceDataArray;
  dataArray->reserve (rows);

  for (p = 0, r = 0; r < rows; r++) {
    QSurfaceDataRow *newRow = new QSurfaceDataRow (cols);
    for (c = 0; c < cols; c++, p++) {
      double yval = 0.0;
      if (is_complex (res, p)) {
	double rv = (double)get_real (res, p);
	double iv = (double)get_imag (res, p);
	switch (curve->getCpx ()) {
	case CPX_REAL:
	  yval = rv;
	  break;
	case CPX_IMAG:
	  yval = iv;
	  break;
	case CPX_MAG:
	  yval = abs (std::complex<double>(rv, iv));
	  break;
	case CPX_PHASE:
	  yval = arg (std::complex<double>(rv, iv));
	  break;
	}
      }
      else yval = (double)get_real (res, p);
      if (y_max < yval) y_max = yval;
      if (y_min > yval) y_min = yval;
      
      (*newRow)[c].setPosition(QVector3D (xvals[c], yval, zvals[r]));
    }
    *dataArray << newRow;
  }

  if (xvals[0] < xvals[xvals.size () - 1]) {
    x_min = xvals[0];
    x_max = xvals[xvals.size () - 1];
  }
  if (zvals[0] < zvals[zvals.size () - 1]) {
    z_min = zvals[0];
    z_max = zvals[zvals.size () - 1];
  }
  
  return dataArray;
}

QAbstractSeries *
ChartWindow::handle_vector (qreal &y_max,
			    qreal &y_min,
			    APL_value res,
			    QVector<double> &xvals,
			    QVector<double> &zvals __attribute__((unused)),
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
	vect[c].imag (0.0);
	break;
      case CPX_MAG:
	vect[c].real (abs (vect[c]));
	vect[c].imag (0.0);
	break;
      case CPX_PHASE:
	vect[c].real (arg (vect[c]));
	vect[c].imag (0.0);
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
      sseries->setPointsVisible (curve->getPointsVisible ());
      sseries->setPointLabelsVisible (curve->getPointLabelsVisible ());
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
  if (idx) {
    QString outString;
    QString errString;
    QString name = idx->getName ();
    if (!name.isEmpty ()) {
      QString cmd =
	QString (")erase %1").arg (name);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
    }
  }
}

QVector<double> 
ChartWindow::setIndex (Index *idx, int incr, QString title)
{
  QString name;
  QVector<double>vals;
  if (idx && !(name = idx->getName ()).isEmpty ()) {
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
  ChartData *cd = chartControls->getChartData ();
  int i;
  char loc[256];
  MainWindow *mw = chartControls->getMainWindow ();
  QString outString;
  QString errString;
  bool polar  = (Qt::Checked == chartControls->do_polar->checkState ());
  bool spline = (Qt::Checked == chartControls->do_spline->checkState ());

  chartControls->getMainWindow ()->setParams ();

  Index *ix = chartControls->getChartData ()->getXIndex ();
  QVector<double> xvals =
    setIndex (ix, mw->getIncr (), chartControls->chart_title->text ());
  
  Index *iz = chartControls->getChartData ()->getZIndex ();
  QVector<double> zvals =
    setIndex (iz, mw->getIncr (), chartControls->chart_title->text ());
  
  QList<int> sels =  chartControls->getChartData ()->getSelected ();
  bool chart_created = false;

  QString curve_label;
  QList<QAbstractSeries *>series_list;
  QList<QSurfaceDataArray *>surface_list;
  qreal x_max = -MAXDOUBLE;
  qreal x_min =  MAXDOUBLE;
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;
  qreal z_max = -MAXDOUBLE;
  qreal z_min =  MAXDOUBLE;
  
	
  for (i =  0; i < sels.size (); i++) {
    Curve curve = mw->getCurve (sels[i]);
    curve_label = curve.getLabel ();		// set to last curve
    QString fcn = curve.getFunction ();
    QString stmt = QString ("%1 ← %2").arg (expvar).arg (fcn);
      
    outString.clear ();
    errString.clear ();
    AplExec::aplExec (APL_OP_EXEC, stmt, outString, errString);  // evaluate
    mw->update_screen (errString, outString);

    if (errString.isEmpty ()) {
      sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
      APL_value res = get_var_value (expvar, loc);
      if (res) {
	if (get_rank (res) == 2) {
	  QSurfaceDataArray *surface =
	    handle_surface (x_max, x_min,
			    y_max, y_min,
			    z_max, z_min,
			    res, xvals, zvals,  &curve);
	  surface_list.append (surface);
	}
	else if (get_rank (res) == 1) {
	  QAbstractSeries *series =
	    handle_vector (y_max, y_min, res, xvals, zvals, spline, &curve);
	  series_list.append (series);
	}
	QString cmd =
	  QString (")erase %1").arg (expvar);
	AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
	sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
	release_value (res, loc);
      }
    }
  }

  if (series_list.size () > 0 && surface_list.size () > 0) {
    QMessageBox msgBox;
    QString msg =
      QString ("Sorry, can't mix 2D and 3d plots.");
    msgBox.setText (msg);
    msgBox.setIcon (QMessageBox::Warning);
    msgBox.exec();
  }
  else {
    if (series_list.size () > 0) {
      
      chartView  = new QChartView ();
      polarchart = new QPolarChart ();
      chart      = new QChart ();
      chartView->setChart (polar ? polarchart : chart);
      chartView->chart ()->setDropShadowEnabled(true);
      chartView->chart ()->setTheme (cd->getTheme ());
      
      chartView->chart ()->removeAllSeries();

      for (i = 0; i < series_list.size (); i++) {
	if (series_list[i]) chartView->chart ()->addSeries (series_list[i]);
	chart_created = true;
      }

      if (chart_created) {
	chartView->setRenderHint (QPainter::Antialiasing);
	chartView->chart ()->setTitle (chartControls->chart_title->text ());

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

	chartView->chart ()->createDefaultAxes ();

	qreal dy = 0.075 * (y_max - y_min);
	chartView->chart ()->axes (Qt::Vertical).first()
	  ->setRange(y_min-dy, y_max+dy);  
  
	QString ix_label = ix->getLabel ();
	chartView->chart ()->axes (Qt::Horizontal).first()
	  ->setTitleText (ix_label);
	chartView->chart ()->axes (Qt::Vertical).first()
	  ->setTitleText (curve_label);

	this->setCentralWidget (chartView);
	this->show ();
      }
    }
    if (surface_list.size () > 0) {
      QSurfaceDataProxy *proxy  = new QSurfaceDataProxy();
      QSurface3DSeries  *series = new QSurface3DSeries(proxy);
      proxy->resetArray (surface_list[0]);

      //DrawWireframe, DrawSurface, DrawSurfaceAndWireframe 
      series->setDrawMode(QSurface3DSeries::DrawSurface);
      //      series->setFlatShadingEnabled(true);

      if (graph) {
	graph->close ();
	delete graph;
      }
      Q3DSurface *graph = new Q3DSurface();
#if 0
      graph->setFlags (graph->flags() ^ Qt::FramelessWindowHint);
      graph->setMinimumSize (QSize (1024, 1024));
      graph->setMaximumSize (QSize (1024, 1024));
      graph->setGeometry (0,0, 1024, 1024);
      glViewport ((GLint) 0, (GLint) 0,
		  (GLsizei)1024, (GLsizei)1024);
#endif
  
      graph->axisX()->setLabelFormat ("%.2f");
      graph->axisZ()->setLabelFormat ("%.2f");
#if 0
      graph->axisX()->setRange ((float)x_min, (float)x_max);
      graph->axisY()->setRange ((float)y_min, (float)y_max);
      graph->axisZ()->setRange ((float)z_min, (float)z_max);
#endif
      graph->axisX()->setLabelAutoRotation (60);
      graph->axisY()->setLabelAutoRotation (45);
      graph->axisZ()->setLabelAutoRotation (60);
      
      glScalef (40.0f, 40.0f, 40.0f);
      
      graph->addSeries (series);

      QWidget *container = QWidget::createWindowContainer(graph);
      this->setCentralWidget (container);
#if 0
      graph->resize (1024, 1024);
      QSize sz = container->size ();
      graph->setMargin ((qreal)0.0);
      fprintf (stderr, "size %d %d %g\n",
	       sz.width (), sz.height (),
	       (double)graph->margin ()
	       );
      QRect    rect = graph->frameGeometry ();
      QMargins marg = graph->frameMargins ();
      QPoint   posn = graph->framePosition ();
      fprintf (stderr, "rect b: %d t: %d l: %d r: %d\n",
	       rect.bottom (), rect.top (), rect.left (), rect.right ());
      fprintf (stderr, "marg b: %d t: %d l: %d r: %d\n",
	       marg.bottom (), marg.top (), marg.left (), marg.right ());
      fprintf (stderr, "posn x: %d y: %d\n", posn.x (), posn.y ());
#endif
      this->show ();
    }
  }

  chartControls->getMainWindow ()->eraseParams ();
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
  graph = nullptr;

  QVariant ww = settings.value (SETTINGS_WIDTH);
  QVariant hh = settings.value (SETTINGS_HEIGHT);
  if (ww.isValid () && hh.isValid ()) 
    this->resize (ww.toInt (), hh.toInt ());

  drawChart ();
  
  //  create_menuBar ();

}

ChartWindow::~ChartWindow()
{

}
