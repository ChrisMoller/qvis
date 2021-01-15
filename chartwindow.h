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
#include <QPolarChart>

QT_CHARTS_USE_NAMESPACE

#include "mainwindow.h"

class MainWindow;
class ChartWindow;

class ChartEnter : public QObject
{
    Q_OBJECT
  
public:
  ChartEnter (QChartView *obj, ChartWindow *cw)
  {watched = obj; chartwin = cw;}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
  ChartWindow *chartwin;
  QChartView *watched;
};




class ChartWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  ChartWindow(MainWindow *parent = nullptr);
  ~ChartWindow();
  QChartView	*chartView;
  QChart	*chart;
  QPolarChart	*polarchart;
  std::vector<Curve> curves;
  Curve		 curve;
  bool		 changed;
  bool 		 saveFile (QString &fileName);
  void 		 readFile (QString &fileName);
  void		 initXmlHash ();
  void		 saveSettings ();
  QSettings 	 settings;

private slots:
  void imageExport();
  void settheme();
  void themeChanged(int newtheme);
  void setfont();
  
public slots:
  void handleExpression ();
  
private:
  QFont titlefont;
  QChart::ChartTheme theme;
  MainWindow	*mainWindow;
  bool parseCurve (Curve &curve, QXmlStreamReader &stream);
  bool parseFunction (Curve &curve, QXmlStreamReader &stream);
  bool parseRange (Range &rng, QXmlStreamReader &stream);
  bool parseIdx (Index &idx, QXmlStreamReader &stream);
  bool parseIx (Curve &curve, QXmlStreamReader &stream);
  bool parseIz (Curve &curve, QXmlStreamReader &stream);
  int	         handle_vector (APL_value res,
				APL_value xvals,
				QString flbl);
  void create_menuBar ();
  QComboBox 	*themebox;
  ChartEnter *chartEnter;
};


#endif  // CHARTWINDOW_H
