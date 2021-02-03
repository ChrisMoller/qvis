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

#ifndef CURVES_H
#define CURVES_H

#include <QtWidgets>

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

class Curve
{
 public:
  Curve (QString &rname, QString &rfcn, int rpen, QColor rcolour)
    {
      name	= rname;
      fcn	= rfcn;
      pen	= rpen;
      colour	= rcolour;
    }

  void showCurve ()
  {
    fprintf (stderr, "%s: %s\n",
	     toCString (name),
	     toCString (fcn));
    fprintf (stderr, "pen: %d\n", pen);
    fprintf (stderr, "colour: %d %d %d\n",
	     colour.red (), colour.green (), colour.blue ());
  }
      

 private:
  QString name;
  QString fcn;
  int pen;
  QColor colour;
};

#endif // CURVES_H
