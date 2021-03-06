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
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>

using namespace QtDataVisualization;

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

enum {
  CPX_REAL,
  CPX_IMAG,
  CPX_MAG,
  CPX_PHASE,
  CPX_PROJ
};

class Param
{
public:
  Param (QString &rname, std::complex<double> rval);
  std::complex<double> getValue ();
  QString getName ();
  void setName (QString &newname);
  void setValue (std::complex<double> &val);
  void setValue (double real, double imag);
  void setReal (double real);
  void setImag (double imag);

private:
  QString name;
  std::complex<double> val;
};

class Curve
{
 public:
  Curve (QString &rname, QString &rlabel,
	 QString &rfcn, Qt::PenStyle rpen, QColor rcolour);
  void 		showCurve ();
  QString 	getName ();
  void 		setName (QString &newname);
  QString 	getLabel ();
  void 		setLabel (QString &newlabel);
  QString	getFunction ();
  void		setFunction (QString &newfunction);
  Qt::PenStyle	getPen ();
  void    	setPen (Qt::PenStyle rpen);
  QString 	getPenName ();
  QColor  	getColour ();
  void    	setColour (QColor rcolour);
  bool  	getPointsVisible ();
  void    	setPointsVisible (bool state);
  bool  	getPointLabelsVisible ();
  void    	setPointLabelsVisible (bool state);
  int  		getCpx ();
  void    	setCpx (int rcpx);
  QSurface3DSeries::DrawFlags getDrawMode ();
  void		setDrawMode (QSurface3DSeries::DrawFlags rdrawmode);
  bool 		getSpline ();
  void    	setSpline (bool rspline);

 private:
  bool    spline;
  QSurface3DSeries::DrawFlags drawMode;
  QString name;			// key 
  QString label;		// y-axis key
  QString fcn;
  Qt::PenStyle pen;
  QColor colour;
  QStringList penNames = {
    "No Pen",
    "Solid Line",
    "Dash Line",
    "Dot Line",
    "Dash Dot Line",
    "Dash DotDot Line"
  };
#if 0
  QFont	  font;
#endif
  bool pointsVisible;
  bool pointLabelsVisible;
  int  cpx;
};

#endif // CURVES_H
