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
    <vis height="." width="." theme=".">
      <curve polar="." spline="."> <!-- any number of repeats -->
        <shorttitle>.....</shorttitle>
        <title>.....</title>
        <function>
	  <label>...</label>
	  <expression>...</expression>
	</function>
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
      </curve>
    </vis>
 ***/

#include <QtWidgets>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
//#include <QHash>

#include "mainwindow.h"
#include "xml.h"

#define xml_def(v, l) #v, XML_ ## v, l
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };

bool
MainWindow::saveFile (QString &fileName)
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
  Qt::CheckState polar_checked = do_polar->checkState();
  stream.writeAttribute(xml_tags[XML_polar].tag,
			(polar_checked == Qt::Checked)  ?
			QString::number (xml_tags[XML_true].logical) :
			QString::number (xml_tags[XML_false].logical));
  Qt::CheckState spline_checked = do_spline->checkState();
  stream.writeAttribute(xml_tags[XML_spline].tag,
			(spline_checked == Qt::Checked)  ?
			QString::number (xml_tags[XML_true].logical) :
			QString::number (xml_tags[XML_false].logical));
  
  stream.writeTextElement(xml_tags[XML_shorttitle].tag, "none");
  stream.writeTextElement(xml_tags[XML_title].tag, chart_title->text ());
  
  stream.writeStartElement(xml_tags[XML_function].tag);
  stream.writeTextElement(xml_tags[XML_label].tag, fcn_label->text ());
  stream.writeTextElement(xml_tags[XML_title].tag, y_title->text ());
  stream.writeTextElement(xml_tags[XML_expression].tag,
			  apl_expression->text ());
  stream.writeEndElement(); // function
  
  stream.writeStartElement (xml_tags[XML_ix].tag);
  stream.writeTextElement(xml_tags[XML_name].tag,  x_var_name->text ());
  stream.writeTextElement(xml_tags[XML_title].tag, x_title->text ());

  stream.writeStartElement(xml_tags[XML_range].tag);
  stream.writeTextElement(xml_tags[XML_min].tag,
			  QString::number (x_var_min->value ()));
  stream.writeTextElement(xml_tags[XML_max].tag,
			  QString::number (x_var_max->value ()));
  stream.writeEndElement(); // range

  stream.writeEndElement(); // ix
  
  stream.writeStartElement(xml_tags[XML_iz].tag);
  stream.writeTextElement(xml_tags[XML_name].tag,  z_var_name->text ());
  stream.writeTextElement(xml_tags[XML_title].tag, z_title->text ());

  stream.writeStartElement(xml_tags[XML_range].tag);
  stream.writeTextElement(xml_tags[XML_min].tag,
			  QString::number (z_var_min->value ()));
  stream.writeTextElement(xml_tags[XML_max].tag,
			  QString::number (z_var_max->value ()));
  stream.writeEndElement(); // range

  stream.writeEndElement(); // iz
  
  stream.writeEndElement(); // curve
  
  stream.writeEndElement(); // qvis
  stream.writeEndDocument();
  file.close ();
  return true;
}

static void
show_curve (Curve &curve)
{
  fprintf (stderr, "polar = %d, spline = %d\n", curve.polar, curve.spline);
  fprintf (stderr, "shorttitle = %s\n",
	   curve.shorttitle.toStdString ().c_str ());
  fprintf (stderr, "curve title = %s\n",
	   curve.title.toStdString ().c_str ());
  fprintf (stderr, "function title = %s\n",
	   curve.function.title.toStdString ().c_str ());
  fprintf (stderr, "function label = %s\n",
	   curve.function.label.toStdString ().c_str ());
  fprintf (stderr, "function expression = %s\n",
	   curve.function.expression.toStdString ().c_str ());
  fprintf (stderr, "ix name = %s\n",
	   curve.ix.name.toStdString ().c_str ());
  fprintf (stderr, "ix title = %s\n",
	   curve.ix.title.toStdString ().c_str ());
  fprintf (stderr, "ix range min = %g\n",
	   curve.ix.range.min);
  fprintf (stderr, "ix range max = %g\n",
	   curve.ix.range.max);
  fprintf (stderr, "iz name = %s\n",
	   curve.iz.name.toStdString ().c_str ());
  fprintf (stderr, "iz title = %s\n",
	   curve.iz.title.toStdString ().c_str ());
  fprintf (stderr, "iz range min = %g\n",
	   curve.iz.range.min);
  fprintf (stderr, "ix range max = %g\n",
	   curve.iz.range.max);
}

bool
MainWindow::parseRange (Range &rng, QXmlStreamReader &stream)
{
  bool rc = true;
  while (stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      if (0 == QString::compare (sn, xml_tags[XML_min].tag))
	rng.min = stream.readElementText ().toDouble ();
      else if (0 == QString::compare (sn, xml_tags[XML_max].tag))
	rng.max = stream.readElementText ().toDouble ();
      else {
	rc = false;
	break;
      }
    }
    else break;
  }
  return rc;
}

bool
MainWindow::parseIdx (Index &idx, QXmlStreamReader &stream)
{
  bool rc = true;
  while (stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      if (0 == QString::compare (sn, xml_tags[XML_name].tag))
	idx.name = stream.readElementText ();
      else if (0 == QString::compare (sn, xml_tags[XML_title].tag))
	idx.title = stream.readElementText ();
      else if (0 == QString::compare (sn, xml_tags[XML_range].tag))
	rc = parseRange (idx.range, stream);
      else {
	rc = false;
	break;
      }
    }
    else break;
  }
  return rc;
}

bool
MainWindow::parseIx (Curve &curve, QXmlStreamReader &stream)
{
  return parseIdx (curve.ix, stream);
}

bool
MainWindow::parseIz (Curve &curve, QXmlStreamReader &stream)
{
  return parseIdx (curve.iz, stream);
}

bool
MainWindow::parseFunction (Curve &curve, QXmlStreamReader &stream)
{
  bool rc = true;
  while (stream.readNextStartElement()) {
    if (stream.isStartElement ()) {
      QString sn = stream.name ().toString ();
      if (0 == QString::compare (sn, xml_tags[XML_label].tag))
	curve.function.label = stream.readElementText ();
      else if (0 == QString::compare (sn, xml_tags[XML_title].tag))
	curve.function.title = stream.readElementText ();
      else if (0 == QString::compare (sn, xml_tags[XML_expression].tag))
	curve.function.expression = stream.readElementText ();
      else {
	rc = false;
	break;
      }
    }
    else break;
  }
  return rc;
}

bool
MainWindow::parseCurve (Curve &curve, QXmlStreamReader &stream)
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
	    if (0 == QString::compare (sn, xml_tags[XML_shorttitle].tag)) 
	      curve.shorttitle = stream.readElementText ();
	    else if (0 == QString::compare (sn, xml_tags[XML_title].tag))
	      curve.title = stream.readElementText ();
	    else if (0 == QString::compare (sn, xml_tags[XML_function].tag))
	      rc = parseFunction (curve, stream);
	    else if (0 == QString::compare (sn, xml_tags[XML_ix].tag))
	      rc = parseIx (curve, stream);
	    else if (0 == QString::compare (sn, xml_tags[XML_iz].tag)) 
	      rc = parseIz (curve, stream);
	    else {
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
MainWindow::readFile (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
  QXmlStreamReader stream(&file);

  Curve curve;
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

  if (rc) show_curve (curve);
  else {
    fprintf (stderr, "Error line %d\n", (int)stream.lineNumber ());
  }
}

