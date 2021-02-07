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


#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

class Index
{
 public:
  Index (QString &rname, QString &rlabel, double rmin, double rmax)
    {
      name  = rname;
      label = rlabel;
      min   = rmin;
      max   = rmax;
    }
  void showIndex ()
  {
    fprintf (stderr, "%s %s min: %g max: %g\n",
	     toCString (name),
	     toCString (label),
	     min, max);
  }
  QString	getName ()  { return name;}
  QString	getLabel () { return label;}
  double	getMin ()   { return min;}
  double	getMax ()   { return max;}
  
 private:
  QString name;
  QString label;
  double  min;
  double  max;
};

class ChartData
{
 public:
  ChartData () {};
  ChartData (QString &rtitle, bool rspline, bool rpolar,
	     Index *rix, Index *riz, QList<int> &rselected)
    {
      title  = rtitle;
      spline = rspline;
      polar  = rpolar;
      ix     = rix;
      iz     = riz;
      selected = rselected;
    }

  void showChart ()
  {
    fprintf (stderr, "%s spline=%s polar=%s\n",
	     toCString (title),
	     (spline ? "true" : "false"),
	     (polar ? "true" : "false"));
    if (ix) ix->showIndex ();
    if (iz) iz->showIndex ();
    if (!selected.isEmpty ()) {
      fprintf (stderr, "Selected: ");
      int k;
      for (k = 0; k < selected.size (); k++)
	fprintf (stderr, "%d ", selected[k]);
      fprintf (stderr, "\n");
    }
  }
  void	  	appendSelected (int i) {selected.append (i);}
  void	  	clearSelected () {selected.clear ();}
  QList<int>	getSelected () { return selected;}
  QString	getTitle ()  { return title;}
  bool		getSpline () { return spline;}
  bool		getPolar ()  { return polar;}
  Index		*getXIndex () { return ix; }
  Index		*getZIndex () { return iz; }

 private:
  QString title;
  bool spline;
  bool polar;
  Index *ix;
  Index *iz;
  QList<int> selected;
};

#endif // CHARTDATA_H
