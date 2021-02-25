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
  
  bool spline = (Qt::Checked == chartControls->do_spline->checkState ());

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
      
    //DrawWireframe, DrawSurface, DrawSurfaceAndWireframe 
    series->setDrawMode(QSurface3DSeries::DrawSurface);
    //      series->setFlatShadingEnabled(true);
      
    graph->addSeries (series);
    surfaces_created = true;
  }
  return surfaces_created;
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
      bool polar  = (Qt::Checked == chartControls->do_polar->checkState ());
      chartView  = new QChartView ();
      polarchart = new QPolarChart ();
      chart      = new QChart ();
      chartView->setChart (polar ? polarchart : chart);
      chartView->chart ()->setDropShadowEnabled(true);
      chartView->chart ()->setTheme (cd->getTheme ());
      
      chartView->chart ()->removeAllSeries();

      chart_created = createCurveList ();

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

      camera->setXRotation (0.0f);
      camera->setYRotation (0.0f);

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

#if 0
          https://doc.qt.io/qt-5/qkeyevent.html

Qt::NoModifier		0x00000000	No modifier key is pressed.

Qt::ShiftModifier	0x02000000	A Shift key on the keyboard is pressed.

Qt::ControlModifier	0x04000000	A Ctrl key on the keyboard is pressed.

Qt::AltModifier		0x08000000	An Alt key on the keyboard is pressed.

Qt::MetaModifier	0x10000000	A Meta key on the keyboard is pressed.

Qt::KeypadModifier	0x20000000	A keypad button is pressed.

Qt::GroupSwitchModifier	0x40000000	X11 only (unless activated on
					Windows by a command line argument).
                                        A Mode_switch key on the keyboard
                                        is pressed.
#endif

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

ChartWindow::ChartWindow  (ChartControls *parent)
  : QMainWindow(parent)
{
  chartControls = parent;
  graph = nullptr;

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
  hslider->setValue(0);
  connect (hslider, &QAbstractSlider::valueChanged, this,
	   [=](int value) {
	     double scale = ((double)value) / 1000;
	     if (keymod == Qt::NoModifier) 
	       camera->setXRotation ((float)(180 * scale));
	     else if (keymod == Qt::ShiftModifier) {
	       QVector3D target = camera->target ();
	       target.setX ((float)scale);
	       camera->setTarget (target);
	     }
	   });

  QSlider *vslider = new QSlider (Qt::Vertical);
  vslider->setMinimum (-1000);
  vslider->setMaximum (1000);
  vslider->setValue(0);
  connect (vslider, &QAbstractSlider::valueChanged, this,
	   [=](int value) {
	     double scale = ((double)value) / 1000;
	     if (keymod == Qt::NoModifier) 
	       camera->setYRotation ((float)(180 * scale));
	     else if (keymod == Qt::ShiftModifier) {
	       QVector3D target = camera->target ();
	       target.setY ((float)scale);
	       camera->setTarget (target);
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
