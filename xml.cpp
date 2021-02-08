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
      <chart spline="." polar="." theme=".'>
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
	<selected>. . .</selected>
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
#include "chartdata.h"
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
      ChartData     *cd = cc->getChartData ();
      
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
      stream.writeAttribute(xml_tags[XML_theme].tag,
			    QString::number (cd->getTheme ()));

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

      QList<int> selList = cd->getSelected ();
      fprintf (stderr, "selList size %d\n", selList.size ());
      if (selList.size () > 0) {
	stream.writeStartElement(xml_tags[XML_selected].tag);
	int j;
	for (j =  0; j < selList.size (); j++) 
	  stream.writeCharacters (" " + QString::number (selList[j]));
	stream.writeEndElement(); // selected
      }
      
      stream.writeEndElement(); // chart
    }
    stream.writeEndElement(); // charts
  }
  
  stream.writeEndElement(); // qvis
  stream.writeEndDocument();
  file.close ();
  return true;
}

Index *
MainWindow::parseIdx (QXmlStreamReader &stream)
{
  bool run = true;
  QString name;
  QString label;
  double min = 0.0;
  double max = 0.0;
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
  
  Index *rx = new Index (name, label, min, max);
  return rx;
}

Index *
MainWindow::parseIx (QXmlStreamReader &stream)
{
  return parseIdx (stream);
}

Index *
MainWindow::parseIz (QXmlStreamReader &stream)
{
  return parseIdx (stream);
}


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
  return rc;
}

bool
MainWindow::parseChart (bool spline, bool polar, int theme,
			QXmlStreamReader &stream)
{
  bool rc = true;
  bool run = true;
  QString title;

  Index *ix = nullptr;
  Index *iz = nullptr;
  QList<int> selected;
  
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
	ix = parseIx (stream);
	break;
      case XML_iz:
	iz = parseIz (stream);
	break;
      case XML_selected:
	{
	  fprintf (stderr, "reading sel\n");
	  QString cstr = stream.readElementText ();
	  QStringList cvec = cstr.trimmed ().split (QRegExp ("\\s+"));
	  fprintf (stderr, "cstr = \"%s\", ct = %d\n",
		   toCString (cstr),  cvec.size ());
	  int k;
	  for (k = 0; k < cvec.size (); k++)
	    selected.append (cvec[k].toInt ());
	  fprintf (stderr, "sc = %d\n", selected.size ());
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

  ChartData *cd = new ChartData (title, spline, polar, theme,
				 ix, iz, selected);
  fprintf (stderr, "sc = %d\n", cd->getSelected ().size ());
  charts.append (cd);

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
	    QStringRef themeref = attrs.value (xml_tags[XML_theme].tag);
	    int theme = themeref.isEmpty ()
	      ? QChart::ChartThemeLight : themeref.toInt ();
	    parseChart ((0 == spline.compare (xml_tags[XML_true].tag)),
			(0 == polar.compare (xml_tags[XML_true].tag)),
			theme, stream);
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

  int i;
#if 0
  for (i = 0; i < curves.size (); i++) {
    fprintf (stderr, "\ncurve %d\n", i);
    curves[i].showCurve ();
  }

  for (i =  0; i < charts.size (); i++) {
    fprintf (stderr, "\nchart %d\n", i);
    charts[i]->showChart ();
  }
#endif

  
  for (i =  0; i < charts.size (); i++) {
    ChartControls *tab1 = nullptr;
    if (tabs->count () > 0) {
      int ix = tabs->count () - 1;
      QWidget *widg = tabs->widget (ix);
      ChartControls *cc = (ChartControls *)widg;
      if (!cc->inUse ()) tab1 = cc;
    }
    tab1->setUseState (true);
    tabs->addTab (tab1, charts[i]->getTitle ());
    if (!tab1) tab1 = new ChartControls (i, this);
  }
}

void
MainWindow::initXmlHash ()
{
  for (long unsigned int i = 0; i < XML_LAST; i++)
    xmlhash.insert (xml_tags[i].tag, (int)i);
}
