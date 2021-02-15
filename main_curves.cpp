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

#include <values.h>
#include "mainwindow.h"
#include "curves.h"

enum {
  COLUMN_NAME,
  COLUMN_LABEL,
  COLUMN_FCN,
  COLUMN_SETTINGS,
  COLUMN_X,
  COLUMN_COUNT
};

void
MainWindow::deleteCurve (int idx)
{
  fprintf (stderr, "deleting %d\n", idx);	// fixme
}

static QComboBox *
linestyleCombo (Qt::PenStyle sel)
{
  QComboBox *linestyle_combo = new QComboBox ();
  linestyle_combo->addItem ("Solid Line",
			    QVariant((int)Qt::SolidLine));
  linestyle_combo->addItem ("Dash Line,",
			    QVariant((int)Qt::DashLine));
  linestyle_combo->addItem ("Dot Line,",
			    QVariant((int)Qt::DotLine));
  linestyle_combo->addItem ("Dash Dot Line,",
			    QVariant((int)Qt::DashDotLine));
  linestyle_combo->addItem ("Dash Dot Dot Line",
			    QVariant((int)Qt::DashDotDotLine));
  int found = linestyle_combo->findData (QVariant ((int)sel));
  if (found != -1)
    linestyle_combo->setCurrentIndex (found);
  
  return linestyle_combo;
}

void
MainWindow::setCurves (int which)
{
  fprintf (stderr, "in sc %d\n", which);
}

void
MainWindow::insertItem (int i, QTableWidget* &curvesTable)
{
  QLineEdit *curve_name = new QLineEdit (curves[i].getName ());
  QObject::connect (curve_name,
		    &QLineEdit::returnPressed,
		    [=]()
		    {QString name = curve_name->text ();
		      curves[i].setName (name);
		      updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_NAME, curve_name);

  QLineEdit *curve_label = new QLineEdit (curves[i].getLabel ());
  QObject::connect (curve_label,
		    &QLineEdit::returnPressed,
		    [=]()
		    {QString label = curve_label->text ();
		      curves[i].setLabel (label);
		      updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_LABEL, curve_label);

  QLineEdit *curve_fcn = new QLineEdit (curves[i].getFunction ());
  QObject::connect (curve_fcn,
		    &QLineEdit::returnPressed,
		    [=]()
		    {QString fcn = curve_fcn->text ();
		      curves[i].setFunction (fcn);
		      updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_FCN, curve_fcn);

#if 1
  QPushButton *settings_pb = new QPushButton (QObject::tr ("Settings"));
  connect (settings_pb,
	   &QAbstractButton::clicked,
	   [=](){setCurves (i);
	    updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_SETTINGS, settings_pb);
#else
  ColorSelector *curve_colour = new ColorSelector ();
  curve_colour->setUpdateMode (ColorSelector::Confirm);
  curve_colour->setColor (curves[i].getColour ());
  QObject::connect (curve_colour,
		    &ColorSelector::colorSelected,
		    [=]()
		    {QColor colour = curve_colour->color ();
		      curves[i].setColour (colour);
		      updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_COLOUR, curve_colour);

  QComboBox *curve_pen =  linestyleCombo (curves[i].getPen ());
  connect (curve_pen, QOverload<int>::of(&QComboBox::activated),
	  [=](int index)
	  {QVariant sel = curve_pen->itemData (index);
	    curves[i].setPen ((Qt::PenStyle)sel.toInt ());
	    updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_PEN, curve_pen);
#endif

  const QIcon deleteIcon = QIcon (":/images/edit-delete.png");
  QPushButton *deleteButton =
    new QPushButton (deleteIcon, QObject::tr ("Delete"));
  connect (deleteButton,
	   &QAbstractButton::clicked,
	   [=](){deleteCurve (i);
	    updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_X, deleteButton);
}

void
MainWindow::addCurve()
{
  QDialog dialog (this, Qt::Dialog);
  QGridLayout *dialog_layout = new QGridLayout;
  dialog.setLayout (dialog_layout);

  /***** existing curves *****/
  
  QGroupBox *gbox = new QGroupBox ("Curves");
  QHBoxLayout *curvesLayout = new QHBoxLayout ();
  gbox->setLayout (curvesLayout);
  dialog_layout->addWidget (gbox);
    
  curvesTable       = new QTableWidget (this);
#if 0
  connect (curvesTable, &QTableWidget::cellPressed,
	   this, &MainWindow::cellPressed);
#endif

#define CH_NAME		"Name"
#define CH_LABEL	"Label"
#define CH_FCN		"Function"
#define CH_SETTINGS	"Settings"
#define CH_DELETE	"Delete"
  curvesTable->setColumnCount (COLUMN_COUNT);
  curvesTable->setRowCount (curves.size ());
  QTableWidgetItem *column_name     = new QTableWidgetItem(CH_NAME);
  QTableWidgetItem *column_label    = new QTableWidgetItem(CH_LABEL);
  QTableWidgetItem *column_fcn      = new QTableWidgetItem(CH_FCN);
  QTableWidgetItem *column_settings = new QTableWidgetItem(CH_SETTINGS);
  QTableWidgetItem *column_x        = new QTableWidgetItem(CH_DELETE);
  QString colour_style_style ("background-color: yellow; color: red;");
  curvesTable->setHorizontalHeaderItem (COLUMN_NAME,	 column_name);
  curvesTable->setHorizontalHeaderItem (COLUMN_LABEL,	 column_label);
  curvesTable->setHorizontalHeaderItem (COLUMN_FCN,	 column_fcn);
  curvesTable->setHorizontalHeaderItem (COLUMN_SETTINGS, column_settings);
  curvesTable->setHorizontalHeaderItem (COLUMN_X,	 column_x);
  int available_width = gbox->width ();
  QFontMetrics fm = QWidget::fontMetrics();
  int name_width     = 60 + fm.horizontalAdvance (CH_NAME);
  available_width -= name_width;
  int label_width    = 20 + fm.horizontalAdvance (CH_LABEL);
  available_width -= label_width;
  int settings_width = 20 + fm.horizontalAdvance (CH_SETTINGS);
  available_width -= settings_width;
  int delete_width   = 20 + fm.horizontalAdvance (CH_DELETE);
  available_width -= delete_width;
  
  curvesTable->setColumnWidth (COLUMN_NAME, 	name_width);
  curvesTable->setColumnWidth (COLUMN_LABEL, 	label_width);
  curvesTable->setColumnWidth (COLUMN_FCN, 	available_width);
  curvesTable->setColumnWidth (COLUMN_SETTINGS, settings_width);
  curvesTable->setColumnWidth (COLUMN_X,	delete_width);
  int i;
  for (i = 0; i < curves.size (); i++)
    insertItem (i, curvesTable);
  curvesLayout->addWidget (curvesTable);

  /***** new curves *****/

  QGroupBox *formGroupBox = new QGroupBox (QString ("New curve"));
  QGridLayout *layout = new QGridLayout ();
  formGroupBox->setLayout (layout);
  dialog_layout->addWidget (formGroupBox);
  
  int row = 0;
  int col = 0;

  QLineEdit *curve_name = new QLineEdit ();
  curve_name->setPlaceholderText ("Curve name");
  layout->addWidget (curve_name, row, col++);

  QLineEdit *curve_label = new QLineEdit ();
  curve_label->setPlaceholderText ("Curve label");
  layout->addWidget (curve_label, row, col++);

  QLineEdit *curve_function = new QLineEdit ();
  curve_function->setPlaceholderText ("Curve function");
  layout->addWidget (curve_function, row, col++);

  ColorSelector curve_colour;
  curve_colour.setUpdateMode (ColorSelector::Confirm);
  layout->addWidget (&curve_colour, row, col++);

  QComboBox *linestyle_combo = linestyleCombo (Qt::SolidLine);
  layout->addWidget (linestyle_combo, row, col++);
  
  row++;
  QPushButton *cancelButton = new QPushButton (QObject::tr ("Close"));
  cancelButton->setAutoDefault (false);
  cancelButton->setDefault (false);
  layout->addWidget (cancelButton, row, 3);
  QObject::connect (cancelButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);
  QPushButton *acceptButton = new QPushButton (QObject::tr ("Accept"));
  acceptButton->setAutoDefault (true);
  acceptButton->setDefault (true);
  layout->addWidget (acceptButton, row, 4);
  QObject::connect (acceptButton, &QPushButton::clicked,
		    &dialog, &QDialog::accept);

  QPoint loc = this->pos ();
  dialog.move (loc.x () + 200, loc.y () + 200);
  bool run = true;
  while(run) {
    curve_name->setFocus ();
    int drc = dialog.exec ();
    
    if (drc == QDialog::Accepted) {
      QString name	= curve_name->text ();
      QString label	= curve_label->text ();
      QString function	= curve_function->text ();
      QVariant pen	= linestyle_combo->currentData ();
      QColor  colour = curve_colour.color ();
      if (!name.isEmpty () && !function.isEmpty ()) {
	Curve   curve = Curve (name, label, function,
			       (Qt::PenStyle)(pen.toInt ()), colour);
	curves.append (curve);
	int nextRow = curvesTable->rowCount();
	curvesTable->setRowCount (1 + nextRow);
	insertItem (nextRow, curvesTable);
      }
      curve_name->clear ();
      curve_label->clear ();
      curve_function->clear ();
#if 0
      for (i = 0; i < curves.size (); i++) {
	QWidget *widget = curvesTable->cellWidget (i, COLUMN_COLOUR);
	QColor colour = ((ColorSelector *)widget)->color ();
	curves[i].setColour (colour);
      }
#endif
#if 0
      {
	int i;
	
	for (i = 0; i < tabs->count (); i++) {
	  QWidget *widg = tabs->widget (i);
	  ChartControls *cc = (ChartControls *)widg;
	  QComboBox *cbox = cc->curves_combo;
	  cbox->clear ();
	  int j;
	  for (j = 0; j < curves.size (); j++) {
	    cbox->addItem (curves[j].getName ());
	  }
	}
      }
#endif
    }
    else run = false;
  }

#if 0
  int j;
  for (j = 0; j < curves.size (); j++) {
    QTableWidgetItem *item = curvesTable->item (j, 0);
    QString name = item->data (Qt::EditRole).toString ();
    QString oldname = curves[j].getName ();
    if (name.compare (oldname))
      curves[j].setName (name);
  }
#endif
}
