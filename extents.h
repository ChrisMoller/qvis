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

#ifndef EXTENTS_H
#define EXTENTS_H


class Extents
{
public:
  Extents ();
  void clear ();
  void setX (double x);
  void setY (double y);
  void setZ (double z);
  void setX (double min, double max);
  void setY (double min, double max);
  void setZ (double min, double max);
  double deltaX ();
  double deltaY ();
  double deltaZ ();
  double sumX ();
  double sumY ();
  double sumZ ();
  double minX ();
  double maxX ();
  double minY ();
  double maxY ();
  double minZ ();
  double maxZ ();
  double offsetX (double x);
  double offsetY (double y);
  double offsetZ (double z);
private:
  qreal x_max;
  qreal x_min;
  qreal y_max;
  qreal y_min;
  qreal z_max;
  qreal z_min;
};

#endif // EXTENTS_H
