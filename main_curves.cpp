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
  COLUMN_COLOUR,
  COLUMN_PEN,
  COLUMN_X
};


void
MainWindow::insertItem (int i, QTableWidget* &curvesTable)
{
#if 1
  QLineEdit *curve_name = new QLineEdit (curves[i].getName ());
  QObject::connect (curve_name,
		    &QLineEdit::returnPressed,
		    [=]()
		    {QString name = curve_name->text ();
		      curves[i].setName (name);
		      updateAll (); notifySelective (false); });
  curvesTable->setCellWidget (i, COLUMN_NAME, curve_name);
#else
  QTableWidgetItem *item_name =
    new QTableWidgetItem (curves[i].getName ());
  item_name->setFlags (Qt::ItemIsEnabled | Qt::ItemIsEditable);
  curvesTable->setItem (i, COLUMN_NAME, item_name);
#endif

  QTableWidgetItem *item_lbl =
    new QTableWidgetItem (curves[i].getLabel ());
  item_lbl->setFlags (Qt::ItemIsEnabled | Qt::ItemIsEditable);
  curvesTable->setItem (i, COLUMN_LABEL, item_lbl);

  QTableWidgetItem *item_fcn =
    new QTableWidgetItem (curves[i].getFunction ());
  item_fcn->setFlags (Qt::ItemIsEnabled | Qt::ItemIsEditable);
  curvesTable->setItem (i, COLUMN_FCN, item_fcn);

  ColorSelector *curve_colour = new ColorSelector ();
  curve_colour->setUpdateMode (ColorSelector::Confirm);
  curve_colour->setColor (curves[i].getColour ());
  curvesTable->setCellWidget (i, COLUMN_COLOUR, curve_colour);

  QTableWidgetItem *item_pen =
    new QTableWidgetItem (curves[i].getPenName ());
  curvesTable->setItem (i, COLUMN_PEN, item_pen);

  QTableWidgetItem *item_delete =
    new QTableWidgetItem (QIcon (":/images/edit-delete.png"), "Delete");
  curvesTable->setItem (i, COLUMN_X, item_delete);
}


static QComboBox *
linestyleCombo (int sel)
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
  linestyle_combo->setCurrentIndex (sel);
  
  return linestyle_combo;
}


void
MainWindow::cellPressed (int row, int column)
{
  switch(column) {
  case COLUMN_X:
    {
      if (row >= 0 && row < curves.size ()) {
	QMessageBox msgBox;
	msgBox.setText ("Did you really mean that?");
	msgBox.setStandardButtons (QMessageBox::Yes |
				   QMessageBox::Cancel);
	msgBox.setDefaultButton (QMessageBox::Cancel);
	QPoint loc = this->pos ();
	msgBox.move (loc.x () + 200, loc.y () + 200);
	if (msgBox.exec() == QMessageBox::Yes) {
	  curves.removeAt (row);
	  curvesTable->removeRow (row);
	}
      }
    }
    break;
  case  COLUMN_PEN:
    {
     QDialog dialog (this, Qt::Dialog);
     QGridLayout *layout = new QGridLayout;
     dialog.setLayout (layout);
     
     QComboBox *linestyle_combo = linestyleCombo (0);
     int penIdx = (int)curves[row].getPen ();
     int found = linestyle_combo->findData (QVariant(penIdx));
     if (found != -1) linestyle_combo->setCurrentIndex (found);
     layout->addWidget (linestyle_combo, 0, 0, 1, 2);

     QPushButton *acceptButton = new QPushButton (QObject::tr ("Accept"));
     acceptButton->setAutoDefault (true);
     acceptButton->setDefault (true);
     layout->addWidget (acceptButton, 1, 1);
     QObject::connect (acceptButton, &QPushButton::clicked,
		       &dialog, &QDialog::accept);
     if (QDialog::Accepted == dialog.exec ()) {
       int pen	= (linestyle_combo->currentData ()).toInt ();
       curves[row].setPen ((Qt::PenStyle)pen);
       QTableWidgetItem *item = curvesTable->item (row, COLUMN_PEN);
       item->setText (curves[row].getPenName ());
     }
    }
    break;
  }
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
  connect (curvesTable, &QTableWidget::cellPressed,
	   this, &MainWindow::cellPressed);
  
  curvesTable->setColumnCount (6);
  curvesTable->setRowCount (curves.size ());
  QTableWidgetItem *column_name   = new QTableWidgetItem(tr("Name"));
  QTableWidgetItem *column_label  = new QTableWidgetItem(tr("Label"));
  QTableWidgetItem *column_fcn    = new QTableWidgetItem(tr("Function"));
  QTableWidgetItem *column_colour = new QTableWidgetItem(tr("Colour"));
  QTableWidgetItem *column_pen    = new QTableWidgetItem(tr("Pen"));
  QTableWidgetItem *column_x      = new QTableWidgetItem(tr("Delete"));
  QString colour_style_style ("background-color: yellow; color: red;");
  curvesTable->setHorizontalHeaderItem (COLUMN_NAME,	column_name);
  curvesTable->setHorizontalHeaderItem (COLUMN_LABEL,	column_label);
  curvesTable->setHorizontalHeaderItem (COLUMN_FCN,	column_fcn);
  curvesTable->setHorizontalHeaderItem (COLUMN_COLOUR,	column_colour);
  curvesTable->setHorizontalHeaderItem (COLUMN_PEN,	column_pen);
  curvesTable->setHorizontalHeaderItem (COLUMN_X,	column_x);
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

  QComboBox *linestyle_combo = linestyleCombo (0);
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
      for (i = 0; i < curves.size (); i++) {
	QWidget *widget = curvesTable->cellWidget (i, COLUMN_COLOUR);
	QColor colour = ((ColorSelector *)widget)->color ();
	curves[i].setColour (colour);
      }
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
