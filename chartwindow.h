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

#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QPolarChart>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QOpenGLFunctions>

#include <math.h>

#if 1
#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#endif


// https://doc.qt.io/qt-5/qopenglfunctions.html#details

QT_CHARTS_USE_NAMESPACE
using namespace QtDataVisualization;

#include "chartdata.h"
#include "mainwindow.h"
#include "chartcontrols.h"
#include "extents.h"

#define INITIAL_X_ROTATION 30.0
#define INITIAL_Y_ROTATION 30.0
#define INITIAL_AZIMUTH -90.0
#define INITIAL_ELEVATION 45.0
#define INITIAL_FOV 30.0

class ChartControls;
class MainWindow;
class ChartWindow;
class CharData;
class Index;

#define DtoR(d)  ((M_PI * (d)) / 360.0)
#define RtoD(r)  ((180.0 * (r)) / M_PI)

class VChartView : public QChartView
{
  Q_OBJECT

public:
  VChartView  (ChartWindow *parent = nullptr) { cw = parent; }
  void drawForeground (QPainter *painter, const QRectF &rect);

private:
  ChartWindow *cw;
};

class ChartEnter : public QObject
{
    Q_OBJECT
  
public:
  ChartEnter (VChartView *obj, ChartWindow *cw, MainWindow *mw)
  {watched = obj; chartwin = cw; mainwin = mw;}

protected:
  //    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  MainWindow  *mainwin;
  ChartWindow *chartwin;
  VChartView  *watched;
};

class ChartFilter : public QObject
{
    Q_OBJECT
  
public:
  ChartFilter (VChartView *obj, QChart *ct, QPolarChart *cp, ChartWindow *cw);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  QChart	*fchart;
  QPolarChart	*fpolarchart;
  ChartWindow 	*fchartwin;
  VChartView	*fwatched;
};

// https://ericlavesson.blogspot.com/2013/03/c-ownership-semantics.html

class ChartWindow : public QMainWindow  //, protected QOpenGLFunctions
{
  Q_OBJECT
  
public:
  //void	       drawForeground (QPainter *painter, const QRectF &rect);
  ChartWindow  (ChartControls *parent = nullptr);
  ~ChartWindow();
  QChart	*chart;
  QPolarChart	*polarchart;
  bool		 changed;
  QSettings 	 settings;
  QWidget 	*drawChart ();
  void 		 reDraw ();
  void 		setContent (Index *&ix, Index *&iz, QString &curve_label,
			    QList<Curve> *curve_list);
  bool		createCurveList ();
  bool		createSurfaceList (Q3DSurface *graph,
				   QList<Curve> &curve_list);
  //  std::vector<OldCurve> curves;
  void exportChart (int width, int height, QString &fn,
		    QChart *mchart, bool isPolar);
  gsl_matrix *	getCX ();
  Extents *getExtents ();
  Extents *getRex ();
  void setCoord ();

private slots:
#if 0
  void imageExport();
  //  void settheme();
  void themeChanged(int newtheme);
  void setfont();
#endif
  
public slots:
  //  void handleExpression ();

protected:  
  VChartView	*chartView;

private:
  ChartFilter *chartFilter;
  bool chartEventFilter(QObject *obj, QEvent *event);
  void closeEvent (QCloseEvent *event);
  QList<QAbstractSeries *>series_list;
  QList<QSurfaceDataArray *>surface_list;
  QFont titlefont;
  QChart::ChartTheme theme;
  ChartControls	*chartControls;
  QAbstractSeries *handle_vector (APL_value res,
				  QVector<double> &xvals,
				  QVector<double> &zvals,
				  Curve *curve);
  QSurfaceDataArray *handle_surface (APL_value res,
				     QVector<double> &xvals,
				     QVector<double> &zvals,
				     Curve *curve);
  QVector<double> setIndex (Index *idx, int incr, QString title);
  void	eraseIndex (Index *idx);
  Q3DSurface *graph;
  Q3DCamera *camera;
  gsl_matrix *hRot;
  gsl_matrix *vRot;
  gsl_matrix *elevationRotation;
  gsl_matrix *azimuthRotation;
  gsl_matrix *perspectiveXform;
  Extents extents;
  Extents rex;
#if 1
  double azimuth;
  double elevation;
#else
  double frustrumAngle;
  double dof;
#endif
  double distance;
  gsl_matrix *createPerspectiveXform ();
  QLabel *coords;
};


#endif  // CHARTWINDOW_H
