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
#include <QCloseEvent>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QOpenGLFunctions>
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
			    Curve *curve)
{
  QString flbl      = curve->getName ();
  QColor  fcolour   = curve->getColour ();
  Qt::PenStyle fpen = curve->getPen ();
  bool spline	    = curve->getSpline ();
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
ChartWindow::setContent (qreal &x_max, qreal &x_min,
			 qreal &y_max, qreal &y_min,
			 qreal &z_max, qreal &z_min,
			 Index *&ix, Index *&iz,
			 QString &curve_label,
			 QList<Curve> *curve_list)
{
  MainWindow *mw = chartControls->getMainWindow ();
  QString outString;
  QString errString;
  char loc[256];
  
  chartControls->getMainWindow ()->setParams ();

  ix = chartControls->getChartData ()->getXIndex ();
  QVector<double> xvals =
    setIndex (ix, mw->getIncr (), chartControls->chart_title->text ());
  
  iz = chartControls->getChartData ()->getZIndex ();
  QVector<double> zvals =
    setIndex (iz, mw->getIncr (), chartControls->chart_title->text ());
  
  QList<int> sels =  chartControls->getChartData ()->getSelected ();

  int i;
  for (i =  0; i < sels.size (); i++) {
    Curve curve = mw->getCurve (sels[i]);
    curve_list->append (curve);
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
	    handle_vector (y_max, y_min, res, xvals, zvals, &curve);
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
}

bool
ChartWindow::createCurveList ()
{
  int i;
  bool chart_created = false;
  for (i = 0; i < series_list.size (); i++) {
    if (series_list[i]) {
      chartView->chart ()->addSeries (series_list[i]);
      chart_created = true;
    }
  }
  return chart_created;
}

bool
ChartWindow::createSurfaceList (Q3DSurface *graph, QList<Curve> &curve_list)
{
  int i;
  bool surfaces_created = false;
  for (i = 0; i < surface_list.size (); i++) {
    QSurfaceDataProxy *proxy  = new QSurfaceDataProxy();
    QSurface3DSeries  *series = new QSurface3DSeries(proxy);
    Curve curve = curve_list[i];
    series->setBaseColor (curve.getColour ());
    proxy->resetArray (surface_list[i]);

    QSurface3DSeries::DrawFlags dm = curve.getDrawMode ();
    // no idea how this is getting set to zero, but to hell
    // with figuring it out
    if ((int) dm == 0) dm = QSurface3DSeries::DrawSurface;
    series->setDrawMode (dm);
    series->setName (QString ("MMMMMMMM"));
      
    graph->addSeries (series);
    surfaces_created = true;
  }
  return surfaces_created;
}

ChartFilter::ChartFilter (QChartView *obj, QChart *ct,
			  QPolarChart *cp, ChartWindow *cw)
{
  fwatched 	= obj;
  fchart	= ct;
  fpolarchart	= cp;
  fchartwin	= cw;
}

bool
ChartFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == fwatched) {
    if (event->type() == QEvent::MouseButtonPress) {
      QSettings settings;
      QMouseEvent *me = (QMouseEvent *)event;
      if (me->button () == Qt::RightButton) {
	QFileDialog dialog (fwatched, QString ("Export chart"));
	dialog.setOption (QFileDialog::DontUseNativeDialog);
	dialog.setAcceptMode (QFileDialog::AcceptSave);
	QString ff = settings.value (SETTINGS_EXPORT_FN).toString ();
	if (!ff.isEmpty ()) dialog.selectFile (ff);
	QLayout *layout = dialog.layout ();
	QGroupBox *gbox = new QGroupBox ();
	QHBoxLayout *btnlayout = new QHBoxLayout ();
	gbox->setLayout (btnlayout);
	QDoubleSpinBox *widthBox  = new QDoubleSpinBox ();
	QDoubleSpinBox *heightBox = new QDoubleSpinBox ();
	widthBox->setMinimum (32.0);
	widthBox->setMaximum (8192.0);
	QString ww = settings.value (SETTINGS_EXPORT_WIDTH).toString ();
	widthBox->setValue (ww.isEmpty ()
			    ? EXPORT_WIDTH_DEFAULT
			    : ww.toFloat ());
	widthBox->setDecimals (0);
	widthBox->setStepType (QAbstractSpinBox::AdaptiveDecimalStepType);
	heightBox->setMinimum (32.0);
	heightBox->setMaximum (8192.0);
	QString hh = settings.value (SETTINGS_EXPORT_HEIGHT).toString ();
	heightBox->setValue (ww.isEmpty ()
			    ? EXPORT_HEIGHT_DEFAULT
			    : ww.toFloat ());
	heightBox->setDecimals (0);
	heightBox->setStepType (QAbstractSpinBox::AdaptiveDecimalStepType);
	btnlayout->addWidget (new QLabel ("Width:"));
	btnlayout->addWidget (widthBox);
	btnlayout->addWidget (new QLabel ("Height:"));
	btnlayout->addWidget (heightBox);
	layout->addWidget (gbox);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	if (dialog.exec() == QDialog::Accepted) {
	  QString fn =  dialog.selectedFiles().first();
	  double width  = widthBox->value ();
	  double height = heightBox->value ();
	  fchartwin->exportChart ((int)width, (int)height, fn,
				 fchart, fpolarchart);
	}
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

void
ChartWindow::exportChart (int width, int height, QString &fn,
			  QChart *mchart, QPolarChart *mpolarchart)
{
  QChartView *lclChartView  = new QChartView ();
  qreal x_max = -MAXDOUBLE;
  qreal x_min =  MAXDOUBLE;
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;

  if (mchart) {
    QRectF pa = mchart->plotArea ();
    qreal scale = 0.6 * ((qreal)height) / pa.height ();
    QChart *nchart = new QChart ();
    nchart->setTheme (mchart->theme ());
    nchart->setDropShadowEnabled (mchart->isDropShadowEnabled ());
    nchart->removeAllSeries();
    QList<QAbstractSeries *> series = mchart->series ();
    int i;
    for (i = 0; i < series.size (); i++) {
      QAbstractSeries *tt = series[i];
      switch (tt->type ()) {
      case QAbstractSeries::SeriesTypeSpline:
	{
	  int j;
	  QLineSeries *ss = (QLineSeries *)series[i];
	  QSplineSeries *sseries = new QSplineSeries ();
	  sseries->setName (ss->name ());
	  sseries->setColor (ss->color ());
	  QPen pen = ss->pen ();
	  pen.setWidthF (scale * ss->pen ().widthF ());
	  sseries->setPen (pen);
	  sseries->setPointsVisible (ss->pointsVisible ());
	  sseries->setPointLabelsVisible (ss->pointLabelsVisible ());
	  nchart->addSeries (sseries);
	  for (j = 0; j < ss->count (); j++) {
	    qreal x_val = ss->at (j).x ();
	    qreal y_val = ss->at (j).y ();
	    if (x_max < x_val) x_max = x_val;
	    if (x_min > x_val) x_min = x_val;
	    if (y_max < y_val) y_max = y_val;
	    if (y_min > y_val) y_min = y_val;
	    sseries->append (ss->at (j));
	  }
	  lclChartView->setChart (nchart);
	}
	break;
      case QAbstractSeries::SeriesTypeLine:
	{
	  int j;
	  QLineSeries *ss = (QLineSeries *)series[i];
	  QLineSeries *sseries = new QSplineSeries ();
	  sseries->setName (ss->name ());
	  sseries->setColor (ss->color ());
	  sseries->setPen (ss->pen ());
	  // sseries->setPointsVisible (curve->getPointsVisible ());
	  // sseries->setPointLabelsVisible (curve->getPointLabelsVisible ());
	  nchart->addSeries (sseries);
	  for (j = 0; j < ss->count (); j++)
	    sseries->append (ss->at (j));
	  lclChartView->setChart (nchart);
	}
	break;
      default:
	break;
      }
    }

    
    nchart->setTitle (mchart->title ());
    QFont fnt = mchart->titleFont ();
    qreal ps = scale * fnt.pointSizeF ();
    fnt.setPointSizeF (ps);
    nchart->setTitleFont (fnt);

    fnt = mchart->legend ()->font ();
    ps = scale * fnt.pointSizeF ();
    fnt.setPointSizeF (ps);
    nchart->legend ()->setFont (fnt);

    nchart->createDefaultAxes ();
    qreal dx = 0.075 * (x_max - x_min);
    qreal dy = 0.075 * (y_max - y_min);
    nchart->axes (Qt::Vertical).first()
      ->setRange(y_min-dy, y_max+dy);
    nchart->axes (Qt::Horizontal).first()
      ->setRange(x_min-dx, x_max+dx);
    
    QString vl = mchart->axes (Qt::Vertical).first()->titleText();
    nchart->axes (Qt::Vertical).first()->setTitleText (vl);
    
    QString hl = mchart->axes (Qt::Horizontal).first()->titleText();
    nchart->axes (Qt::Horizontal).first()->setTitleText (vl);
    
    fnt = mchart->axes (Qt::Vertical).first()->labelsFont ();
    ps = 0.8 * scale * fnt.pointSizeF ();
    fnt.setPointSizeF (ps);
    nchart->axes (Qt::Vertical).first()->setLabelsFont (fnt);
    
    fnt = mchart->axes (Qt::Horizontal).first()->labelsFont ();
    ps = 0.8  * scale * fnt.pointSizeF ();
    fnt.setPointSizeF (ps);
    nchart->axes (Qt::Horizontal).first()->setLabelsFont (fnt);
    {
      QString fn = chartControls->getChartData ()->getBGFile ();
      if (!fn.isEmpty ()) {
	QImage gep (fn);
	gep.scaled (lclChartView->width (), lclChartView->height (),
		    Qt::KeepAspectRatioByExpanding,
		    Qt::SmoothTransformation);
	QBrush  brush (gep);
	nchart->setBackgroundBrush (brush);
      }
    }
  }
  //  lclChartView->setChart (mchart ?: mpolarchart);
  lclChartView->setGeometry (0, 0, width, height);
  lclChartView->setRenderHint (QPainter::Antialiasing);

  QPixmap p = lclChartView->grab();
  if (p.save(fn)) {
    QSettings settings;
    settings.setValue (QString (SETTINGS_EXPORT_WIDTH),  QVariant (width));
    settings.setValue (QString (SETTINGS_EXPORT_HEIGHT), QVariant (height));
    settings.setValue (QString (SETTINGS_EXPORT_FN), 	 QVariant (fn));
  }
  else {
    QMessageBox msgBox;
    QString msg = QString ("Image format not supported");
    msgBox.setText (msg);
    msgBox.setIcon (QMessageBox::Warning);
    msgBox.exec();
  }
}

QWidget *
ChartWindow::drawChart ()
{
  QWidget *widg = nullptr;
  ChartData *cd = chartControls->getChartData ();
  QString outString;
  QString errString;
  qreal x_max = -MAXDOUBLE;
  qreal x_min =  MAXDOUBLE;
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;
  qreal z_max = -MAXDOUBLE;
  qreal z_min =  MAXDOUBLE;
  Index *ix = nullptr;
  Index *iz = nullptr;
  QString curve_label;
  QList<Curve> curve_list;

  /****  content  ****/

  setContent (x_max, x_min,
	      y_max, y_min,
	      z_max, z_min,
	      ix, iz, curve_label, &curve_list);

  bool chart_created = false;
  
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
      bool polar  = cd->getPolar ();
      chartView   = new QChartView ();
      polarchart  = nullptr;
      chart       = nullptr;

      if (polar) {
	polarchart  = new QPolarChart ();
	chartView->setChart (polarchart);
      }
      else {
	chart  = new QChart ();
	chartView->setChart (chart);
      }

      chartFilter = new ChartFilter (chartView, chart, polarchart, this);
      chartView->installEventFilter (chartFilter);
      chartView->chart ()->setDropShadowEnabled (true);
      chartView->chart ()->setTheme (cd->getTheme ());
      
      chartView->chart ()->removeAllSeries();

      chart_created = createCurveList ();

      if (chart_created) {
	chartView->setRenderHint (QPainter::Antialiasing);
	chartView->chart ()->setTitle (chartControls->chart_title->text ());
	chartView->chart ()->setTitleFont (cd->getFont ());
	chartView->chart ()->legend ()->setFont (cd->getLegendFont ());

	QString fn = chartControls->getChartData ()->getBGFile ();
	if (!fn.isEmpty ()) {
	  QImage gep (fn);
	  gep.scaled (chartView->width (), chartView->height (),
		      Qt::KeepAspectRatioByExpanding,
		      Qt::FastTransformation);
	  QBrush  brush (gep);
	  chartView->chart ()->setBackgroundBrush (brush);
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
	
	widg = chartView;
      }
    }
    if (surface_list.size () > 0) {
      if (graph) {
	graph->close ();
	//delete graph;
      }
      graph = new Q3DSurface();

      /*bool surfacesCreated = */
      createSurfaceList (graph, curve_list);

      graph->axisX()->setRange ((float)x_min, (float)x_max);
      graph->axisY()->setRange ((float)y_min, (float)y_max);
      graph->axisZ()->setRange ((float)z_min, (float)z_max);
      graph->setTitle (QString ("MMMMMMMM"));

      /****** end of content *****/
      
      graph->axisX()->setLabelFormat ("%.2f");
      graph->axisZ()->setLabelFormat ("%.2f");
      graph->axisX()->setLabelAutoRotation (60);
      graph->axisY()->setLabelAutoRotation (45);
      graph->axisZ()->setLabelAutoRotation (60);

      Q3DScene *scene = graph->scene ();
      // https://doc.qt.io/qt-5/q3dcamera-members.html
      // see also activeLight, etc
      camera = scene->activeCamera ();
      camera->setZoomLevel (125.0f);

      camera->setXRotation ((float)INITIAL_X_ROTATION);
      camera->setYRotation ((float)INITIAL_Y_ROTATION);

      QWidget *container = QWidget::createWindowContainer(graph);
      container->setMinimumSize(640, 512);
      container->setSizePolicy(QSizePolicy::Expanding,
			       QSizePolicy::Expanding);
      container->setFocusPolicy(Qt::StrongFocus);

      widg = container;
    }
  }

  chartControls->getMainWindow ()->eraseParams ();
  eraseIndex (ix);
  eraseIndex (iz);

  return widg;
}


// https://doc.qt.io/qt-5/qkeyevent.html

static Qt::KeyboardModifiers keymod = Qt::NoModifier;

void
QWidget::keyPressEvent(QKeyEvent *event)
{
  keymod =  event->modifiers();
}

void
QWidget::keyReleaseEvent(QKeyEvent *event)
{
  keymod =  event->modifiers();
}

void
ChartWindow::reDraw  ()
{
  qreal x_max = -MAXDOUBLE;
  qreal x_min =  MAXDOUBLE;
  qreal y_max = -MAXDOUBLE;
  qreal y_min =  MAXDOUBLE;
  qreal z_max = -MAXDOUBLE;
  qreal z_min =  MAXDOUBLE;
  Index *ix = nullptr;
  Index *iz = nullptr;
  QString curve_label;
  QList<Curve> curve_list;
  
  ChartData *cd = chartControls->getChartData ();

  series_list.clear ();
  surface_list.clear ();
  setContent (x_max, x_min,
	      y_max, y_min,
	      z_max, z_min,
	      ix, iz, curve_label, &curve_list);

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
      bool polar  = cd->getPolar ();
      chartView->setChart (polar ? polarchart : chart);
      chartView->chart ()->setDropShadowEnabled(true);
      chartView->chart ()->setTheme (cd->getTheme ());
      chartView->chart ()->setTitleFont (cd->getFont ());
      chartView->chart ()->setTitle (chartControls->chart_title->text ());
      chartView->chart ()->legend ()->setFont (cd->getLegendFont ());

      QString fn = cd->getBGFile ();
      if (!fn.isEmpty ()) {
	QImage gep (fn);
	gep.scaled (chartView->width (), chartView->height (),
		    Qt::KeepAspectRatioByExpanding,
		    Qt::FastTransformation);
	QBrush  brush (gep);
	chartView->chart ()->setBackgroundBrush (brush);
      }
      
      chartView->chart ()->removeAllSeries();
      bool chart_created = createCurveList ();
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
    }
    if (surface_list.size () > 0) {
      int i;
      QList<QSurface3DSeries *>slist = graph->seriesList();
      for (i = 0; i < slist.size (); i++)
	graph->removeSeries (slist[i]);
      createSurfaceList (graph, curve_list);
      graph->axisX()->setRange ((float)x_min, (float)x_max);
      graph->axisY()->setRange ((float)y_min, (float)y_max);
      graph->axisZ()->setRange ((float)z_min, (float)z_max);
    }
  }
}

void
ChartWindow::closeEvent (QCloseEvent *event __attribute__((unused)))
{
  MainWindow *mw = chartControls->getMainWindow ();
  QTabWidget *tabs = mw->getTabs ();
  int i;

  ChartData *cd = nullptr;
  for (i = 0; i < tabs->count (); i++) {
    QWidget *widg = tabs->widget (i);
    if (widg == chartControls) {
      tabs->removeTab (i);
      cd = chartControls-> getChartData ();
      delete cd;
      delete chartControls;
      break;
    }
  }

  if (cd) {
    QList<ChartData*> *charts = mw->getCharts ();
    for (i = 0; i < charts->size (); i++) {
      if (cd == (*charts)[i]) {
	charts->removeAt (i);
	break;
      }
    }
  }
}

ChartWindow::ChartWindow  (ChartControls *parent)
  : QMainWindow(parent)
{
  chartControls = parent;
  graph = nullptr;
  camera = nullptr;
    
#if 0
  QVariant ww = settings.value (SETTINGS_WIDTH);
  QVariant hh = settings.value (SETTINGS_HEIGHT);
  if (ww.isValid () && hh.isValid ()) 
    this->resize (ww.toInt (), hh.toInt ());
#endif
  
  QGroupBox   *outerGroupBox = new QGroupBox ();
  QGridLayout *outerLayout = new QGridLayout ();
  outerGroupBox->setLayout (outerLayout);

  QSlider *hslider = new QSlider (Qt::Horizontal);
  hslider->setMinimum (-1000);
  hslider->setMaximum (1000);
  hslider->setValue(1000.0 * INITIAL_X_ROTATION / 180.0);
  connect (hslider, &QAbstractSlider::valueChanged, this,
	   [=](int value) {
	     double scale = ((double)value) / 1000;
	     if (camera && keymod == Qt::NoModifier) 
	       camera->setXRotation ((float)(180 * scale));
	     else if (camera && keymod == Qt::ShiftModifier) {
	       QVector3D target = camera->target ();
	       target.setX ((float)scale);
	       camera->setTarget (target);
	     }
	   });

  QSlider *vslider = new QSlider (Qt::Vertical);
  vslider->setMinimum (-1000);
  vslider->setMaximum (1000);
  vslider->setValue(1000.0 * INITIAL_Y_ROTATION / 180.0);
  connect (vslider, &QAbstractSlider::valueChanged, this,
	   [=](int value) {
	     double scale = ((double)value) / 1000;
	     if (camera && keymod == Qt::NoModifier) {
	       camera->setYRotation ((float)(90 * scale));
	       fprintf (stderr, "rot = %g\n", 90.0 * scale);
	     }
	     else if (camera && keymod == Qt::ShiftModifier) {
	       QVector3D target = camera->target ();
	       target.setY ((float)scale);
	       camera->setTarget (target);
	     }
	     else if (camera && keymod == Qt::ControlModifier) {
	       double minz  = (double)camera->minZoomLevel ();
	       double maxz  = (double)camera->maxZoomLevel ();
	       double avg   = (minz + maxz) / 2.0;
	       double delta = (maxz - minz) / 2.0;
	       double zoom  = avg + scale * delta;
	       camera->setZoomLevel ((float)zoom);
	     }
	   });

  outerLayout->addWidget (hslider, 0, 0);
  outerLayout->addWidget (vslider, 1, 1);
  
  QWidget *chart = drawChart ();
  if (chart) outerLayout->addWidget (chart, 1, 0);

  this->setCentralWidget (outerGroupBox);
  this->show ();


  
  //  create_menuBar ();

}

ChartWindow::~ChartWindow()
{
}
