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

#include "curves.h"

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

Curve::Curve (QString &rname, QString &rlabel,
       QString &rfcn, int rpen, QColor rcolour)
{
  name	= rname;
  label	= rlabel;
  fcn	= rfcn;
  pen	= rpen;
  colour	= rcolour;
}

void
Curve::showCurve ()
{
  fprintf (stderr, "\"%s\": \"%s\" \"%s\"\n",
	   toCString (name),
	   toCString (label),
	   toCString (fcn));
  fprintf (stderr, "pen: %d\n", pen);
  fprintf (stderr, "colour: %d %d %d\n",
	   colour.red (), colour.green (), colour.blue ());
}

QString
Curve::getName ()
{
  return name;
}

void
Curve::setName (QString &newname)
{
  name = newname;
}

QString
Curve::getLabel ()
{
  return label;
}

QString
Curve::getFunction ()
{
  return fcn;
}

int
Curve::getPen ()
{
  return pen;
}

QColor
Curve::getColour ()
{
  return colour;
}
  
void
Curve::setColour (QColor rcolour)
{
  colour = rcolour;
}

void
Curve::setPen (int rpen)
{
  pen = rpen;
}

QString
Curve::getPenName ()
{
  QString name ("Unknown");
  if (pen >=0 && pen < penNames.size ()) 
    name = penNames[pen];
  return name;
}

