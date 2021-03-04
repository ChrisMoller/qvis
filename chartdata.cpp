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

void
Index::setMin (double v)
{
  min = v;
}

void
Index::setMax (double v)
{
  max = v;
}


void
Index::setName (const QString &text)
{
  name = text;
}

void
Index::setLabel (const QString &text)
{
  label = text;
}

void
ChartData::setWindow (ChartWindow *win)
{
  chartWindow = win;
}

ChartWindow *
ChartData::getWindow ()
{
  return chartWindow;
}

QString
ChartData::getBGFile ()
{
  return backgroundFile;
}

void
ChartData::setBGFile (QString &fn)
{
  backgroundFile = fn;
}

void
ChartData::clearBGFile ()
{
  backgroundFile.clear ();
}

ChartData::ChartData ()
{
  QString ixs = QString ("ix");
  QString izs = QString ("iz");
  ix = new Index (ixs, ixs, 0.0, 0.0);
  iz = new Index (izs, izs, 0.0, 0.0);
  chartWindow = nullptr;
  theme = QChart::ChartThemeQt;
}

ChartData::ChartData (QString &rtitle, bool rpolar, int rtheme,
	   Index *rix, Index *riz, QList<int> &rselected)
{
  title  = rtitle;
  polar  = rpolar;
  ix     = rix;
  iz     = riz;
  selected = rselected;
  theme = (QChart::ChartTheme)rtheme;
  changed = false;
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

void
ChartData::setSelected (int idx, int state)
{
  bool exists = selected.contains (idx);
  if (state == Qt::Checked && !exists) selected.append (idx);
  else if (state != Qt::Checked && exists)
    while (selected.removeOne (idx));
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
ChartData::setTheme (int rtheme)
{
  theme = (QChart::ChartTheme)rtheme;
}

QChart::ChartTheme
ChartData::getTheme ()
{
  return theme;
}

void
ChartData::setChanged (bool state)
{
  changed = state;
}

bool
ChartData::hasChanged ()
{
  return changed;
}

void
ChartData::setUpdate (bool state)
{
  do_update = state;
  if (state) changed = true;
}

bool
ChartData::needsUpdate ()
{
  return do_update;
}

void
ChartData::setPolar (bool rpolar)
{
  polar = rpolar;
}

bool
ChartData::getPolar ()
{
  return polar;
}

void
ChartData::setFont (QFont rfont)
{
  font = rfont;
}

QFont
ChartData::getFont ()
{
  return font;
}
