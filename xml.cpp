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
  <qvis>
    <curves>
      <curve idx=".">
        <name>.</name>
        <label>.</label>
        <function>.</function>
        <pen idx=".">.</pen>
        <colour red=".' green="." blue = "."/>
      </curve>
    </curves>
    <charts>
      <chart spline="." polar=".">
        <title>.</title>
        <ix>
          <name>.</name>
          <label>.</label>
	  <range min="." max="."/>
        </ix>
        <iz>
          <name>.</name>
          <label>.</label>
	  <range min="." max="."/>
        </iz>
      </chart>
    </charts>
  </qvis>
 ***/

#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QHash>

#include "mainwindow.h"
#include "chartwindow.h"
#include "chartcontrols.h"
#include "curves.h"
#include "xml.h"

#define xml_def(v, l) #v, XML_ ## v, l
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };

static QHash<const QString, int> xmlhash;

bool
MainWindow::writeVis (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::WriteOnly | QIODevice::Text);
  QXmlStreamWriter stream(&file);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  stream.writeStartElement(xml_tags[XML_qvis].tag);
  
  if (curves.size () > 0) {
    stream.writeStartElement(xml_tags[XML_curves].tag);
    int i;
    for (i = 0; i < curves.size (); i++) {
      stream.writeStartElement(xml_tags[XML_curve].tag);
      stream.writeAttribute(xml_tags[XML_idx].tag, QString::number (i));
    
      stream.writeTextElement(xml_tags[XML_name].tag,
			      curves[i].getName ());
    
      stream.writeTextElement(xml_tags[XML_label].tag,
			      curves[i].getLabel ());
    
      stream.writeTextElement(xml_tags[XML_function].tag,
			      curves[i].getFunction ());
    
      stream.writeStartElement(xml_tags[XML_pen].tag);
      stream.writeAttribute(xml_tags[XML_idx].tag,
			    QString::number (curves[i].getPen ()));
      stream.writeCharacters(curves[i].getPenName ());
      stream.writeEndElement(); // pen
    
      stream.writeStartElement(xml_tags[XML_colour].tag);
      stream.writeAttribute(xml_tags[XML_red].tag,
			    QString::number (curves[i].getColour ().red ()));
      stream.writeAttribute(xml_tags[XML_green].tag,
			    QString::number (curves[i].getColour ().green ()));
      stream.writeAttribute(xml_tags[XML_blue].tag,
			    QString::number (curves[i].getColour ().blue ()));
      stream.writeCharacters(curves[i].getColour ().name ());
      stream.writeEndElement(); // colour
    
      stream.writeEndElement(); // curve
    }
    stream.writeEndElement(); // curves
  }

  if (tabs->count () > 0) {
    stream.writeStartElement(xml_tags[XML_charts].tag);
    int i;
    for (i = 0; i < tabs->count (); i++) {
      QWidget *widg = tabs->widget (i);
      ChartControls *cc = (ChartControls *)widg; 
      
      stream.writeStartElement(xml_tags[XML_chart].tag);
      bool spline =  (Qt::Checked == cc->do_spline->checkState());
      stream.writeAttribute(xml_tags[XML_spline].tag,
			    QString (spline
				     ? xml_tags[XML_true].tag
				     : xml_tags[XML_false].tag));
      bool polar =  (Qt::Checked == cc->do_polar->checkState());
      stream.writeAttribute(xml_tags[XML_polar].tag,
			    QString (polar
				     ? xml_tags[XML_true].tag
				     : xml_tags[XML_false].tag));

      stream.writeTextElement(xml_tags[XML_title].tag,
			      cc->chart_title->text ());
      
      stream.writeStartElement(xml_tags[XML_ix].tag);
      stream.writeTextElement(xml_tags[XML_name].tag,
			      cc->x_var_name->text ());
      stream.writeTextElement(xml_tags[XML_label].tag,
			      cc->x_label->text ());
      stream.writeStartElement(xml_tags[XML_range].tag);
      stream.writeAttribute(xml_tags[XML_min].tag,
			    QString::number (cc->x_var_min->value ()));
      stream.writeAttribute(xml_tags[XML_max].tag,
			    QString::number (cc->x_var_max->value ()));
      stream.writeEndElement(); // range
      stream.writeEndElement(); // ix
      
      stream.writeStartElement(xml_tags[XML_iz].tag);
      stream.writeTextElement(xml_tags[XML_name].tag,
			      cc->z_var_name->text ());
      stream.writeTextElement(xml_tags[XML_label].tag,
			      cc->z_label->text ());
      stream.writeStartElement(xml_tags[XML_range].tag);
      stream.writeAttribute(xml_tags[XML_min].tag,
			    QString::number (cc->z_var_min->value ()));
      stream.writeAttribute(xml_tags[XML_max].tag,
			    QString::number (cc->z_var_max->value ()));
      stream.writeEndElement(); // range
      stream.writeEndElement(); // ix
      
      stream.writeEndElement(); // chart
    }
    stream.writeEndElement(); // charts
  }

#if 0
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

#endif
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
MainWindow::parseIdx (QXmlStreamReader &stream)
{
#if 1
  bool rc = true;
  bool run = true;
  QString name;
  QString label;
  double min;
  double max;
  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext ();
    QString sn = stream.name ().toString ();
    switch(tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_name:
	name = stream.readElementText ();
	break;
      case XML_label:
	label = stream.readElementText ();
	break;
      case XML_range:
	{
	  QXmlStreamAttributes attrs = stream.attributes();
	  if (!attrs.isEmpty ()) {
	    min = (attrs.value (xml_tags[XML_min].tag)).toDouble ();
	    max = (attrs.value (xml_tags[XML_max].tag)).toDouble ();
	    stream.readElementText ();
	  }
	}
	break;
      }
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    default:
      break;
    }
  }
  fprintf (stderr, "ii %s %s %g %g\n",
	   toCString (name), toCString (label), min, max);

      
#else
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
#endif
  return rc;
}

bool
MainWindow::parseIx (QXmlStreamReader &stream)
{
  return parseIdx (stream);
}

bool
MainWindow::parseIz (QXmlStreamReader &stream)
{
  return parseIdx (stream);
}


#if 0
bool
ChartWindow::parseFunction (OldCurve &curve, QXmlStreamReader &stream)
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
#endif

bool
MainWindow::parseCurve (int idx, QXmlStreamReader &stream)
{
  bool rc = true;
  bool run = true;
  QString name;
  QString label;
  QString function;
  int     pen = 0;
  QColor  colour;
  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext ();
    QString sn = stream.name ().toString ();
    switch(tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_name:
	name = stream.readElementText ();
	break;
      case XML_label:
	label = stream.readElementText ();
	break;
      case XML_function:
	function = stream.readElementText ();
	break;
      case XML_pen:
	{
	  QXmlStreamAttributes attrs = stream.attributes();
	  if (!attrs.isEmpty ())
	    pen = (attrs.value (xml_tags[XML_idx].tag)).toInt ();
	  stream.readElementText ();
	}
	break;
      case XML_colour:
	{
	  QXmlStreamAttributes attrs = stream.attributes();
	  int red   = 0;
	  int green = 0;
	  int blue  = 0;
	  if (!attrs.isEmpty ()) {
	    red   = (attrs.value (xml_tags[XML_red].tag)).toInt ();
	    green = (attrs.value (xml_tags[XML_green].tag)).toInt ();
	    blue  = (attrs.value (xml_tags[XML_blue].tag)).toInt ();
	  }
	  stream.readElementText ();
	  //	  stream.readNext ();		// throw away the </colour>
	  colour.setRgb (red, green, blue);
	}
	break;
      }
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    default:
      break;
    }
  }

  Curve   curve = Curve (name, label, function, pen, colour);
  curves.insert (idx, curve);
  return rc;
}

bool
MainWindow::parseCurves (QXmlStreamReader &stream)
{
  bool rc = false;
  bool run = true;
  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext ();
    QString sn = stream.name ().toString ();
    switch (tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_curve:
	QXmlStreamAttributes attrs = stream.attributes();
	if (!attrs.isEmpty ()) {
	  int idx = (attrs.value (xml_tags[XML_idx].tag)).toInt ();
	  parseCurve (idx, stream);
	}
	break;
      }
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    default:
      break;
    }
  }
#if 1
  int i;
  for (i = 0; i < curves.size (); i++) {
    fprintf (stderr, "\ncurve %d\n", i);
    curves[i].showCurve ();
  }
#endif
  return rc;
}

bool
MainWindow::parseChart (bool spline, bool polar, QXmlStreamReader &stream)
{
  bool rc = true;
  bool run = true;
  QString title;
#if 0
  QString x_name;
  QString x_label;
  double  x_min;
  double  x_max;
  QString z_name;
  QString z_label;
  double  z_min;
  double  z_max;
#endif

  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext ();
    QString sn = stream.name ().toString ();
    switch(tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_title:
	title = stream.readElementText ();
	break;
      case XML_ix:
	parseIx (stream);
	break;
      case XML_iz:
	parseIz (stream);
	break;
      }
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    default:
      break;
    }
  }

  fprintf (stderr, "s = %d, p = %d\n", spline, polar);
  fprintf (stderr, "title = %s\n", toCString(title));
  //  Curve   curve = Curve (name, label, function, pen, colour);
  //  curves.insert (idx, curve);
  return rc;
}

bool
MainWindow::parseCharts (QXmlStreamReader &stream)
{
  bool rc = false;
  bool run = true;
  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext ();
    QString sn = stream.name ().toString ();
    switch (tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_chart:
	{
	  QXmlStreamAttributes attrs = stream.attributes();
	  if (!attrs.isEmpty ()) {
	    QString spline =
	      (attrs.value (xml_tags[XML_spline].tag)).toString ();
	    QString polar  =
	      ((attrs.value (xml_tags[XML_polar].tag))).toString ();
	    parseChart (spline.compare (xml_tags[XML_true].tag),
			polar.compare (xml_tags[XML_true].tag),
			stream);
	  }
	}
	break;
      }
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    default:
      break;
    }
  }
  return rc;
}

#if 0
      QXmlStreamAttributes attrs = stream.attributes();
      if (!attrs.isEmpty ()) {
	int height = (attrs.value (xml_tags[XML_height].tag)).toInt ();
	int width  = (attrs.value (xml_tags[XML_width].tag)).toInt ();
	int theme  = (attrs.value (xml_tags[XML_theme].tag)).toInt ();
	fprintf (stderr, "height = %d, width = %d, theme = %d\n",
	       height, width, theme);
	rc = parseCurves (curve, stream);
      }
      else rc = false;
#endif

void
MainWindow::readVis (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
  QXmlStreamReader stream(&file);

  bool run = true;
  while (run) {
    QXmlStreamReader::TokenType tt = stream.readNext();
    QString sn = stream.name ().toString ();
    switch (tt) {
    case QXmlStreamReader::StartElement:
      switch (xmlhash.value (sn)) {
      case XML_curves:
	parseCurves (stream);
	break;
      case XML_charts:
	parseCharts (stream);
	break;
      }
      break;
    case QXmlStreamReader::EndElement:
      run = false;
      break;
    case QXmlStreamReader::EndDocument:
      run = false;
      break;
    case QXmlStreamReader::Invalid:
      break;
    default:
      break;
    }
  }
}

void
MainWindow::initXmlHash ()
{
  for (long unsigned int i = 0; i < XML_LAST; i++)
    xmlhash.insert (xml_tags[i].tag, (int)i);
}
