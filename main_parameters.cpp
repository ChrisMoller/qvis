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
#include "chartwindow.h"
#include "aplexec.h"



enum {
  PCOLUMN_NAME,
  PCOLUMN_REAL,
  PCOLUMN_IMAG
};

#if 0
void
MainWindow::parmsCellPressed (int row, int column)
{
}
#endif

int
MainWindow::getIncr ()
{
  return incr;
}

void
MainWindow::eraseParams ()
{
  int i;
  for (i = 0; i < parms.size (); i++) {
    QString outString;
    QString errString;
    Param parm = parms[i];
    QString vbl = parm.getName ();
    if (!vbl.isEmpty ()) {
      QString cmd =
	QString (")erase %1").arg (vbl);
      AplExec::aplExec (APL_OP_EXEC, cmd, outString, errString);
    }
  }
}

void
MainWindow::setParams ()
{
  int i;
  char loc[256];

  sprintf (loc, "qvis %s:%d", __FILE__, __LINE__);
  for (i = 0; i < parms.size (); i++) {
    Param parm = parms[i];
    QString vbl = parm.getName ();
    if (!vbl.isEmpty ()) {
      double real = parm.getValue ().real ();
      double imag = parm.getValue ().imag ();

      APL_value res = apl_scalar (loc); 
      if (imag == 0.0)
	set_double ((APL_Float) real, res, 0);
      else
	set_complex ((APL_Float) real, (APL_Float) imag, res, 0);
      
      QByteArray vblUtf8 = vbl.toUtf8();
      int src = set_var_value (vblUtf8.constData (), res, loc);
      if (src != 0) {
	QMessageBox msgBox;
	QString msg = QString ("Error setting parameter %1").arg (vbl);
	msgBox.setText (msg);
	msgBox.setIcon (QMessageBox::Warning);
	msgBox.exec();
      }
      release_value (res, loc);
    }
  }
}

void
MainWindow::insertParmItem (int i, QTableWidget* &parmsTable)
{
  QTableWidgetItem *item_name =
    new QTableWidgetItem (parms[i].getName ());
  item_name->setFlags (Qt::ItemIsEnabled | Qt::ItemIsEditable);
  parmsTable->setItem (i, PCOLUMN_NAME, item_name);

  QDoubleSpinBox *item_real = new QDoubleSpinBox ();
  item_real->setAccelerated (true);
  item_real->setRange (-MAXDOUBLE, MAXDOUBLE);
  item_real->setValue (parms[i].getValue ().real ());
  parmsTable->setCellWidget (i, PCOLUMN_REAL, item_real);
  connect (item_real, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	  [=](double d){parms[i].setReal (d); notifySelective (true);  });

  QDoubleSpinBox *item_imag = new QDoubleSpinBox ();
  item_imag->setAccelerated (true);
  item_imag->setRange (-MAXDOUBLE, MAXDOUBLE);
  item_imag->setValue (parms[i].getValue ().imag ());
  parmsTable->setCellWidget (i, PCOLUMN_IMAG, item_imag);
  connect (item_imag, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	  [=](double d){parms[i].setImag (d); notifySelective (true);  });
}

void
MainWindow::addParms()
{
  QDialog dialog (this, Qt::Dialog);
  QGridLayout *dialog_layout = new QGridLayout;
  dialog.setModal (false);
  dialog.setLayout (dialog_layout);

  /***** existing parms *****/
  
  QGroupBox *gbox = new QGroupBox ("Parameters");
  QHBoxLayout *parmsLayout = new QHBoxLayout ();
  gbox->setLayout (parmsLayout);
  dialog_layout->addWidget (gbox);
    
  parmsTable = new QTableWidget (this);
#if 0
  connect (parmsTable, &QTableWidget::cellPressed,
	   this, &MainWindow::parmsCellPressed);
#endif
  
  parmsTable->setColumnCount (3);
  parmsTable->setRowCount (parms.size ());
  QTableWidgetItem *column_name  = new QTableWidgetItem(tr("Name"));
  QTableWidgetItem *column_real  = new QTableWidgetItem(tr("Real"));
  QTableWidgetItem *column_imag  = new QTableWidgetItem(tr("Imag"));
  parmsTable->setHorizontalHeaderItem (PCOLUMN_NAME,	column_name);
  parmsTable->setHorizontalHeaderItem (PCOLUMN_REAL,	column_real);
  parmsTable->setHorizontalHeaderItem (PCOLUMN_IMAG,	column_imag);
  int i;
  for (i = 0; i < parms.size (); i++)
    insertParmItem (i, parmsTable);
  parmsLayout->addWidget (parmsTable);
  
   /***** new parms *****/

  QGroupBox *formGroupBox = new QGroupBox (QString ("New parameter"));
  QGridLayout *layout = new QGridLayout ();
  formGroupBox->setLayout (layout);
  dialog_layout->addWidget (formGroupBox);
  
  int row = 0;
  int col = 0;
  
  QLineEdit *parm_name = new QLineEdit ();
  parm_name->setPlaceholderText ("Parameter name");
  layout->addWidget (parm_name, row, col++);
  
  QDoubleSpinBox *parm_real = new QDoubleSpinBox ();
  parm_real->setAccelerated (true);
  parm_real->setRange (-MAXDOUBLE, MAXDOUBLE);
  layout->addWidget (parm_real, row, col++);
  
  QDoubleSpinBox *parm_imag = new QDoubleSpinBox ();
  parm_imag->setAccelerated (true);
  parm_imag->setRange (-MAXDOUBLE, MAXDOUBLE);
  layout->addWidget (parm_imag, row, col++);


  row++;
  QPushButton *cancelButton = new QPushButton (QObject::tr ("Close"));
  cancelButton->setAutoDefault (false);
  cancelButton->setDefault (false);
  layout->addWidget (cancelButton, row, 1);
  QObject::connect (cancelButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);
  QPushButton *acceptButton = new QPushButton (QObject::tr ("Accept"));
  acceptButton->setAutoDefault (true);
  acceptButton->setDefault (true);
  layout->addWidget (acceptButton, row, 2);
  QObject::connect (acceptButton, &QPushButton::clicked,
		    &dialog, &QDialog::accept);

  QPoint loc = this->pos ();
  dialog.move (loc.x () + 200, loc.y () + 200);
  bool run = true;
  while(run) {
    parm_name->setFocus ();
    int drc = dialog.exec ();
    if (drc == QDialog::Accepted) {
      QString name = parm_name->text ();
      double  real = parm_real->value ();
      double  imag = parm_imag->value ();
      std::complex<double> val (real, imag);
      if (!name.isEmpty ()) {
	Param parm = Param (name, val);
	parms.append (parm);
	int nextRow = parmsTable->rowCount();
	parmsTable->setRowCount (1 + nextRow);
	insertParmItem (nextRow, parmsTable);
      }
      parm_name->clear ();
    }
    else run = false;
  }
  
  int j;
  for (j = 0; j < parms.size (); j++) {
    QTableWidgetItem *item = parmsTable->item (j, 0);
    QString name = item->data (Qt::EditRole).toString ();
    QString oldname = parms[j].getName ();
    if (name.compare (oldname))
      parms[j].setName (name);
  }
}
