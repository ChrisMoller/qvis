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
/***
  <qvis height="." width=".">
    <curve id="." polar="." spline="."> <!-- any number of repeats -->
      <shorttitle>.....</shorttitle>
      <title>.....</title>
      <function>
        <label>...</label>
	<expression>...</expression>
      </function>
    </curve>
    <chart>		<!-- any number of repeats -->
      <ix>
	<label>...</label>
	<var>...</var>
	<range>
	  <min>...</min>
	  <max>...</max>
	</range>
      </ix>
      <iz>
	<label>...</label>
	<var>...</var>
	<range>
	  <min>...</min>
	  <max>...</max>
	</range>
      </iz>
      <parameter>		<!-- any number of repeats -->
        <var>...</var>
	<value>...</value>
      </parameter>
      <curves>		<!-- any number of repeats -->
        <id>.</id>
        <id>.</id>
        <id>.</id>
      </curves>
    </chart>
  </qvis>
 ***/

#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHash>

#include "mainwindow.h"
#include "chartwindow.h"
#include "xml.h"

#define xml_def(v, l) #v, XML_ ## v, l
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };

static QHash<const QString, int> xmlhash;

bool
ChartWindow::saveFile (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::WriteOnly | QIODevice::Text);
  QXmlStreamWriter stream(&file);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  stream.writeStartElement(xml_tags[XML_qvis].tag);
  stream.writeAttribute(xml_tags[XML_height].tag,
			QString::number (chartView->height ()));
  stream.writeAttribute(xml_tags[XML_width].tag,
			QString::number (chartView->width ()));
  stream.writeAttribute(xml_tags[XML_theme].tag,
			QString::number ((int)theme));
  
  stream.writeStartElement(xml_tags[XML_curve].tag);
  stream.writeAttribute(xml_tags[XML_polar].tag,
			curve.polar ?
			QString::number (xml_tags[XML_true].logical) :
			QString::number (xml_tags[XML_false].logical));
  stream.writeAttribute(xml_tags[XML_spline].tag,
			curve.spline ?
			QString::number (xml_tags[XML_true].logical) :
			QString::number (xml_tags[XML_false].logical));
  
  stream.writeTextElement(xml_tags[XML_shorttitle].tag, curve.shorttitle);
  stream.writeTextElement(xml_tags[XML_title].tag, curve.title);
  
  stream.writeStartElement(xml_tags[XML_function].tag);
  stream.writeTextElement(xml_tags[XML_label].tag,
			  curve.function.label);
  stream.writeTextElement(xml_tags[XML_title].tag,
			  curve.function.title);
  stream.writeTextElement(xml_tags[XML_expression].tag,
			  curve.function.expression);
  stream.writeEndElement(); // function
  
  stream.writeStartElement (xml_tags[XML_ix].tag);
#if 0
  stream.writeTextElement(xml_tags[XML_name].tag,  curve.ix.name);
#endif
  stream.writeTextElement(xml_tags[XML_title].tag, curve.ix.title);

  stream.writeStartElement(xml_tags[XML_range].tag);
  stream.writeTextElement(xml_tags[XML_min].tag,
			  QString::number (curve.ix.range.min));
  stream.writeTextElement(xml_tags[XML_max].tag,
			  QString::number (curve.ix.range.max));
  stream.writeEndElement(); // range

  stream.writeEndElement(); // ix
  
  stream.writeStartElement(xml_tags[XML_iz].tag);
#if 0
  stream.writeTextElement(xml_tags[XML_name].tag,  curve.iz.name);
#endif
  stream.writeTextElement(xml_tags[XML_title].tag, curve.iz.title);

  stream.writeStartElement(xml_tags[XML_range].tag);
  stream.writeTextElement(xml_tags[XML_min].tag,
			  QString::number (curve.iz.range.min));
  stream.writeTextElement(xml_tags[XML_max].tag,
			  QString::number (curve.iz.range.max));
  stream.writeEndElement(); // range

  stream.writeEndElement(); // iz
  
  stream.writeEndElement(); // curve
  
  stream.writeEndElement(); // qvis
  stream.writeEndDocument();
  file.close ();
  return true;
}

bool
ChartWindow::parseRange (Range &rng, QXmlStreamReader &stream)
{
  bool rc = true;
  while (rc && stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      switch (xmlhash.value (sn)) {
	case XML_min:
	  rng.min = stream.readElementText ().toDouble ();
	  break;
	case XML_max:
	  rng.max = stream.readElementText ().toDouble ();
	  break;
	default:
	  rc = false;
	  break;
      }
    }
    else break;
  }
  return rc;
}

bool
ChartWindow::parseIdx (Index &idx, QXmlStreamReader &stream)
{
  bool rc = true;
  while (rc && stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      switch (xmlhash.value (sn)) {
#if 0
      case XML_name:
	idx.name = stream.readElementText ();
	break;
#endif
      case XML_title:
	idx.title = stream.readElementText ();
	break;
      case XML_range:
	rc = parseRange (idx.range, stream);
	break;
      default:
	rc = false;
	break;
      }
    }
    else break;
  }
  return rc;
}

bool
ChartWindow::parseIx (Curve &curve, QXmlStreamReader &stream)
{
  return parseIdx (curve.ix, stream);
}

bool
ChartWindow::parseIz (Curve &curve, QXmlStreamReader &stream)
{
  return parseIdx (curve.iz, stream);
}

bool
ChartWindow::parseFunction (Curve &curve, QXmlStreamReader &stream)
{
  bool rc = true;
  while (rc && stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      switch (xmlhash.value (sn)) {
      case XML_label:
	curve.function.label = stream.readElementText ();
	break;
      case XML_title:
	curve.function.title = stream.readElementText ();
	break;
      case XML_expression:
	curve.function.expression = stream.readElementText ();
	break;
      default:
	rc = false;
	break;
      }
    }
    else break;
  }
  return rc;
}

bool
ChartWindow::parseCurve (Curve &curve, QXmlStreamReader &stream)
{
  bool rc = true;
  stream.readNextStartElement();
  if (stream.isStartElement ()) {
    QString sn = stream.name ().toString ();
    if (0 == QString::compare (sn, xml_tags[XML_curve].tag)) {
      QXmlStreamAttributes attrs = stream.attributes();
      if (!attrs.isEmpty ()) {
	curve.polar  = (attrs.value (xml_tags[XML_polar].tag)).toInt ();
	curve.spline = (attrs.value (xml_tags[XML_spline].tag)).toInt ();
	while (stream.readNextStartElement()) {
	  if (stream.isStartElement ()) {
	    sn = stream.name ().toString ();
	    switch (xmlhash.value (sn)) {
	    case XML_shorttitle:
	      curve.shorttitle = stream.readElementText ();
	      break;
	    case XML_title:
	      curve.title = stream.readElementText ();
	      break;
	    case XML_function:
	      rc = parseFunction (curve, stream);
	      break;
	    case XML_ix:
	      rc = parseIx (curve, stream);
	      break;
	    case XML_iz:
	      rc = parseIz (curve, stream);
	      break;
	    default:
	      rc = false;
	      break;
	    }
	  }
	  else break;
	}
      }
      else rc = false;
    }
  }
  else rc = false;

  return rc;
}

void
ChartWindow::readFile (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
  QXmlStreamReader stream(&file);

  //  Curve curve;
  bool rc = true;

  stream.readNextStartElement();
  if (stream.isStartElement ()) {
    QString sn = stream.name ().toString ();
    if (0 == QString::compare (sn, xml_tags[XML_qvis].tag)) {
      QXmlStreamAttributes attrs = stream.attributes();
      if (!attrs.isEmpty ()) {
	int height = (attrs.value (xml_tags[XML_height].tag)).toInt ();
	int width  = (attrs.value (xml_tags[XML_width].tag)).toInt ();
	int theme  = (attrs.value (xml_tags[XML_theme].tag)).toInt ();
	fprintf (stderr, "height = %d, width = %d, theme = %d\n",
	       height, width, theme);
	rc = parseCurve (curve, stream);
      }
      else rc = false;
    }
  }
  else rc = false;

    // show_curve (curve);
  if (rc)  handleExpression ();
  else { // fixme
    fprintf (stderr, "Error line %d\n", (int)stream.lineNumber ());
  }
}

void
ChartWindow::initXmlHash ()
{
  for (long unsigned int i = 0; i < XML_LAST; i++)
    xmlhash.insert (xml_tags[i].tag, (int)i);
}
