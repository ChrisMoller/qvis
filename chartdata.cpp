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

#include <QtWidgets>
#include "chartdata.h"

Index::Index (QString &rname, QString &rlabel, double rmin, double rmax)
{
  name  = rname;
  label = rlabel;
  min   = rmin;
  max   = rmax;
}

void
Index::showIndex ()
{
  fprintf (stderr, "%s %s min: %g max: %g\n",
	   toCString (name),
	   toCString (label),
	   min, max);
}

QString
Index::getName ()
{
  return name;
}

QString
Index::getLabel ()
{
  return label;
}

double
Index::getMin ()
{
  return min;
}

double
Index::getMax ()
{
  return max;
}

ChartData::ChartData (QString &rtitle, bool rspline, bool rpolar, int rtheme,
	   Index *rix, Index *riz, QList<int> &rselected)
{
  title  = rtitle;
  spline = rspline;
  polar  = rpolar;
  ix     = rix;
  iz     = riz;
  selected = rselected;
  theme = (QChart::ChartTheme)rtheme;
}


void
ChartData::showChart ()
{
  fprintf (stderr, "%s spline=%s polar=%s theme = %d\n",
	   toCString (title),
	   (spline ? "true" : "false"),
	   (polar ? "true" : "false"), (int)theme);
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

void
ChartData::appendSelected (int i)
{
  selected.append (i);
}

void
ChartData::clearSelected ()
{
  selected.clear ();
}

QList<int>
ChartData::getSelected ()
{
  return selected;
}

QString
ChartData::getTitle ()
{
  return title;
}

bool
ChartData::getSpline () {
  return spline;
}

bool
ChartData::getPolar ()
{
  return polar;
}

Index *
ChartData::getXIndex ()
{
  return ix;
}

Index *
ChartData::getZIndex ()
{
  return iz;
}

void
ChartData::setTheme ( int rtheme)
{
  theme = (QChart::ChartTheme)rtheme;
  fprintf (stderr, "settheme %d\n", theme);
}

QChart::ChartTheme
ChartData::getTheme ()
{
  return theme;
}
