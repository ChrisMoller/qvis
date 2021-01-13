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
#include <complex>
#include <values.h>

#include <apl/libapl.h>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"
#include "chartwindow.h"
#include "xml.h"

#define expvar "expvarλ"

int
ChartWindow::handle_vector (APL_value res,
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

    mainWindow->settings.setValue (DO_SPLINE,  curve.spline);
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
ChartWindow::handleExpression ()
{
  mainWindow->settings.setValue (DO_POLAR, curve.polar);
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
      mainWindow->settings.setValue (Z_VAR_NAME, curve.iz.name);
      mainWindow->settings.setValue (Z_VAR_MIN,  curve.iz.range.min);
      mainWindow->settings.setValue (Z_VAR_MAX,  curve.iz.range.max);
      QString range_z =
	QString ("%1 ← (%2) + ((⍳%3+1)-⎕io) × (%4 - %2) ÷ %3")
	.arg(curve.iz.name).arg(curve.iz.range.min)
	.arg(incr).arg(curve.iz.range.max);
      zset = true;
      apl_exec (range_z.toStdString ().c_str ());
    }

    QString input = mainWindow->apl_expression->text ();
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
	mainWindow->settings.setValue (X_VAR_NAME, curve.ix.name);
	mainWindow->settings.setValue (X_VAR_MIN,  curve.ix.range.min);
	mainWindow->settings.setValue (X_VAR_MAX,  curve.ix.range.max);
	mainWindow->settings.setValue (FCN_LABEL,  curve.function.title);
	mainWindow->settings.setValue (FUNCTION,   input);

	mainWindow->settings.setValue (HEIGHT, chartView->height ());
	mainWindow->settings.setValue (WIDTH, chartView->width ());

	mainWindow->settings.setValue (THEME,  theme);
	chartView->chart ()->setTheme (theme);

	mainWindow->settings.setValue (CHART_TITLE,  curve.title);
	chartView->chart ()->setTitle (curve.title);
      
	//	QString x_ttl = x_title->text ();
	mainWindow->settings.setValue (X_TITLE,  curve.ix.title);
	chartView->chart ()->axes (Qt::Horizontal).first()
	  ->setTitleText(curve.ix.title);

	mainWindow->settings.setValue (Y_TITLE,  curve.function.label);
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


ChartWindow::ChartWindow (MainWindow *parent)
  : QMainWindow(parent)
{
  mainWindow = parent;
  initXmlHash ();
  chart      = new QChart ();
  polarchart = new QPolarChart ();
  chartView = new QChartView ();
  chartView->setRenderHint (QPainter::Antialiasing);
  this->setCentralWidget (chartView);
  fprintf (stderr, "aboutto show\n");
  this->show ();

  curves.push_back (curve);
  //chartView->setChart (curve.polar ? polarchart : chart);
  //  handleExpression ();
}

ChartWindow::~ChartWindow()
{

}
