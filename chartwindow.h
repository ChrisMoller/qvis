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

// https://doc.qt.io/qt-5/qopenglfunctions.html#details

QT_CHARTS_USE_NAMESPACE
using namespace QtDataVisualization;

#include "chartdata.h"
#include "mainwindow.h"
#include "chartcontrols.h"

#define INITIAL_X_ROTATION 30.0
#define INITIAL_Y_ROTATION 30.0

class ChartControls;
class MainWindow;
class ChartWindow;
class CharData;
class Index;

class ChartEnter : public QObject
{
    Q_OBJECT
  
public:
  ChartEnter (QChartView *obj, ChartWindow *cw, MainWindow *mw)
  {watched = obj; chartwin = cw; mainwin = mw;}

protected:
  //    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  MainWindow  *mainwin;
  ChartWindow *chartwin;
  QChartView  *watched;
};

class ChartWindow : public QMainWindow  //, protected QOpenGLFunctions
{
  Q_OBJECT
  
public:
  ChartWindow  (ChartControls *parent = nullptr);
  ~ChartWindow();
  QChartView	*chartView;
  QChart	*chart;
  QPolarChart	*polarchart;
  //  OldCurve		 curve;
  bool		 changed;
  QSettings 	 settings;
  QWidget 	*drawChart ();
  void 		 reDraw ();
  void 		setContent (qreal &x_max, qreal &x_min,
			    qreal &y_max, qreal &y_min,
			    qreal &z_max, qreal &z_min,
			    Index *&ix, Index *&iz, QString &curve_label,
			    QList<Curve> *curve_list);
  bool		createCurveList ();
  bool		createSurfaceList (Q3DSurface *graph,
				   QList<Curve> &curve_list);
  //  std::vector<OldCurve> curves;

private slots:
  void imageExport();
#if 0
  //  void settheme();
  void themeChanged(int newtheme);
  void setfont();
#endif
  
public slots:
  //  void handleExpression ();
  
private:
  QList<QAbstractSeries *>series_list;
  QList<QSurfaceDataArray *>surface_list;
  QFont titlefont;
  QChart::ChartTheme theme;
  ChartControls	*chartControls;
  QAbstractSeries *handle_vector (qreal &y_max,
				  qreal &y_min,
				  APL_value res,
				  QVector<double> &xvals,
				  QVector<double> &zvals,
				  Curve *curve);
  QSurfaceDataArray *handle_surface (qreal &x_max, qreal &x_min,
				     qreal &y_max, qreal &y_min,
				     qreal &z_max, qreal &z_min,
				     APL_value res,
				     QVector<double> &xvals,
				     QVector<double> &zvals,
				     Curve *curve);
  QVector<double> setIndex (Index *idx, int incr, QString title);
  void	eraseIndex (Index *idx);
  Q3DSurface *graph;
  Q3DCamera *camera;
#if 0
  void create_menuBar ();
  QComboBox 	*themebox;
  ChartEnter *chartEnter;
#endif
};


#endif  // CHARTWINDOW_H
