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
#include <complex>

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

class Param
{
public:
  Param (QString &rname, std::complex<double> rval);
  std::complex<double> getValue ();
  QString getName ();
  void setName (QString &newname);

private:
  QString name;
  std::complex<double> val;
};

class Curve
{
 public:
  Curve (QString &rname, QString &rlabel,
	 QString &rfcn, int rpen, QColor rcolour);
  void showCurve ();
  QString getName ();
  void setName (QString &newname);
  QString getLabel ();
  QString getFunction ();
  int     getPen ();
  QColor  getColour ();
  void    setColour (QColor rcolour);
  void    setPen (int rpen);
  QString getPenName ();

 private:
  QString name;			// key 
  QString label;		// y-axis key
  QString fcn;
  int pen;
  QColor colour;
  QStringList penNames = {
    "No Pen",
    "Solid Line",
    "Dash Line",
    "Dot Line",
    "Dash Dot Line",
    "Dash DotDot Line"
  };
};

#endif // CURVES_H
