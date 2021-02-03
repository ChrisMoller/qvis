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

class Curve
{
 public:
  Curve (QString &rname, QString &rfcn, QPen rpen, QColor rcolour)
    {
      name	= rname;
      fcn	= rfcn;
      pen	= rpen;
      colour	= rcolour;
    }
  Curve (QString &rname, QString &rfcn)
    {
      name	= rname;
      fcn	= rfcn;
    }
      

 private:
  QString name;
  QString fcn;
  QPen pen;
  QColor colour;
};

#endif // CURVES_H
