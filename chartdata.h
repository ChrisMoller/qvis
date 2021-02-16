/***
    qvis Copyright (C) 2021  Dr. C. H. L. Moller

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

#ifndef CHARTDATA_H
#define CHARTDATA_H

#include <QtWidgets>
#include <QChart>
#include <QtCharts/QChartView>

#include "curves.h"
#include "chartwindow.h"

QT_CHARTS_USE_NAMESPACE

class ChartWindow;

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

class Index
{
 public:
  Index (QString &rname, QString &rlabel, double rmin, double rmax);
  void 		showIndex ();
  QString	getName ();
  QString	getLabel ();
  double	getMin ();
  double	getMax ();
  void		setName (const QString &text);
  void		setLabel (const QString &text);
  void		setMin (double v);
  void		setMax (double v);
  
 private:
  QString name;
  QString label;
  double  min;
  double  max;
};

class ChartData
{
 public:
  ChartData (QString &rtitle, bool rspline, bool rpolar, int rtheme,
	     Index *rix, Index *riz, QList<int> &rselected);

  void 		showChart ();
  void	  	appendSelected (int i);
  void	  	clearSelected ();
  QList<int>	getSelected ();
  void		setSelected (int idx, int state);
  QString	getTitle ();
  bool		getSpline ();
  bool		getPolar ();
  Index		*getXIndex ();
  Index		*getZIndex ();
  QChart::ChartTheme getTheme ();
  void		setTheme (int rtheme);
  void		setWindow (ChartWindow *win);
  ChartWindow  *getWindow ();
  void	        setChanged (bool state);
  bool	        hasChanged ();
  void	        setUpdate (bool state);
  bool	        needsUpdate ();
  QString 	getBGFile ();
  void          setBGFile (QString &fn);
  void 	 	clearBGFile ();

 private:
  QString title;
  bool spline;
  bool polar;
  Index *ix;
  Index *iz;
  QList<int> selected;
  QChart::ChartTheme theme;
  ChartWindow *chartWindow;
  bool 	       changed;
  bool 	       do_update;
  QString      backgroundFile;
};

#endif // CHARTDATA_H
