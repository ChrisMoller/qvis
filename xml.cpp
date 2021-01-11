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
#include <QHash>

#include "mainwindow.h"
#include "xml.h"

#define xml_def(v,p,l) #v, nullptr, XML_ ## v, l
xml_tag_s xml_tags[] = {
#include "XMLtags.def"
  };

static QHash<const QString, int> xmlhash;

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

void
MainWindow::handle_qvis (QXmlStreamReader &stream)
{
  if (stream.isStartElement ()) {
    QXmlStreamAttributes attrs = stream.attributes();
    if (!attrs.isEmpty ()) {
      int height = (attrs.value (xml_tags[XML_height].tag)).toInt ();
      int width  = (attrs.value (xml_tags[XML_width].tag)).toInt ();
      int theme  = (attrs.value (xml_tags[XML_theme].tag)).toInt ();
      fprintf (stderr, "height = %d, width = %d, theme = %d\n",
	       height, width, theme);
    }
  }
  else {
    //    fprintf (stderr, "ending ");
  }
}

void
MainWindow::handle_curve (QXmlStreamReader &stream)
{
  if (stream.isStartElement ()) {
    QXmlStreamAttributes attrs = stream.attributes();
    if (!attrs.isEmpty ()) {
      bool polar  = (attrs.value (xml_tags[XML_polar].tag)).toInt ();
      bool spline = (attrs.value (xml_tags[XML_spline].tag)).toInt ();
      fprintf (stderr, "polar = %d, spline = %d\n",
	       polar, spline);
    }
  }
  else {
    //    fprintf (stderr, "ending ");
  }
}

void
MainWindow::handle_shorttitle (QXmlStreamReader &stream)
{
  if (stream.isStartElement ()) {
    QString st = stream.readElementText ();
    fprintf (stderr, "shorttitle = %s\n", st.toStdString ().c_str ());
  }
  else {
    //    fprintf (stderr, "ending ");
  }
}

void
MainWindow::handle_title (QXmlStreamReader &stream)
{
  if (stream.isStartElement ()) {
    QString st = stream.readElementText ();
    fprintf (stderr, "title = %s\n", st.toStdString ().c_str ());
  }
  else {
    //    fprintf (stderr, "ending ");
  }
}

void
MainWindow::readFile (QString &fileName)
{
  QFile file (fileName);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
  QXmlStreamReader stream(&file);

  while (!stream.atEnd()) {
    stream.readNextStartElement();
    int idx = xmlhash.value (stream.name ().toString (), -1);
    if (idx >= 0 && idx < XML_LAST && xml_tags[idx].handler) 
      (*xml_tags[idx].handler)(stream);
#if 1
    else fprintf (stderr, "%s skipped\n",
		  stream.name ().toString ().toStdString ().c_str ());
#endif
  }
}

void
MainWindow::initXmlHash ()
{
#undef xml_def
#define xml_def(v,p, l) p
  //  static void *fcn[] = {
  void (*fcn[])(QXmlStreamReader &stream) = {
#include "XMLtags.def"
  };
  
  for (long unsigned int i = 0; i < XML_LAST; i++) {
    xmlhash.insert (xml_tags[i].tag, (int)i);
    xml_tags[i].handler = fcn[i]; // (void *)(&handle_qvis);
  }
}
