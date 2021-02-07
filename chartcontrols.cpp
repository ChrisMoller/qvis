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
#include <iostream>
#include <sstream>
#include <values.h>

#include "chartcontrols.h"
#include "curves.h"

//QT_CHARTS_USE_NAMESPACE


void
ChartControls::selectCurves ()
{
  QDialog dialog (this, Qt::Dialog);
  QVBoxLayout *dialog_layout = new QVBoxLayout;
  dialog.setLayout (dialog_layout);
  
  QTableWidget *curvesTable = new QTableWidget (mainWindow);
  curvesTable->setColumnCount (1);
  curvesTable->setRowCount (mainWindow->getCurveCount ());

  QTableWidgetItem *column_label    = new QTableWidgetItem(tr("Label"));
  curvesTable->setHorizontalHeaderItem (0, column_label);

  int i;
  for (i = 0; i < mainWindow->getCurveCount (); i++) {
    QTableWidgetItem *item_lbl =
      new QTableWidgetItem (mainWindow->getCurve (i).getLabel ());
    bool active;
    if (chartData)
      active = chartData->getSelected ().contains (i);
    else active = false;
    item_lbl->setCheckState(active ? Qt::Checked : Qt::Unchecked);
    curvesTable->setItem (i, 0, item_lbl);
  }

  dialog_layout->addWidget (curvesTable);

  QPushButton *closeButton = new QPushButton (QObject::tr ("Accept"));
  dialog_layout->addWidget (closeButton);
  QObject::connect (closeButton, &QPushButton::clicked,
		    &dialog, &QDialog::accept);

  QPoint loc = mainWindow->pos ();
  dialog.move (loc.x () + 200, loc.y () + 200);
  if (QDialog::Accepted == dialog.exec ()) {
    selected.clear ();
    for (i = 0; i < mainWindow->getCurveCount (); i++) {
      QTableWidgetItem *item =curvesTable->item (i, 0);
      Qt::CheckState cs = item->checkState ();
      if (cs == Qt::Checked) selected.append (i);
    }
  }
}

ChartControls::ChartControls (int index, MainWindow *parent)
  : QWidget(parent)
{
  mainWindow = parent;
  tabIndex = index;
  chartData = mainWindow->getChart (tabIndex);
  // /old_home/Qt/Examples/Qt-5.15.1/widgets/dialogs/tabdialog/tabdialog.cpp
  QGridLayout *layout = new QGridLayout ();
  
  int row = 0;
  int col = 0;

  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  
  layout->addWidget (chart_title, row, 0, 1, 2);
  QObject::connect (chart_title,
		    &QLineEdit::editingFinished,
		    this,
		    &ChartControls::titleChangedv);

  col +=2; 

  QPushButton *curvesButton = new QPushButton (QObject::tr ("Curves"));
  layout->addWidget (curvesButton, row, col);
  QObject::connect (curvesButton, SIGNAL (clicked ()),
		    this, SLOT (selectCurves ()));
  row++;
  col = 0;

  /*  x indep vbl */
  
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  layout->addWidget (x_var_name, row, col++);
  
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_min->setToolTip ("x minimum value");
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &ChartControls::valChanged);
  layout->addWidget (x_var_min, row, col++);

  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_max->setToolTip ("x maximum value");
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &ChartControls::valChanged);
  layout->addWidget (x_var_max, row, col++);
  
  x_label = new  QLineEdit ();
  x_label->setPlaceholderText ("x axix label");
  layout->addWidget (x_label, row, col++);

  /*  z indep vbl */

  row++;
  col = 0;

  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  layout->addWidget (z_var_name, row, col++);

  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_min->setToolTip ("z minimum value");
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &ChartControls::valChanged);
  layout->addWidget (z_var_min, row, col++);

  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_max->setToolTip ("z maximum value");
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    &ChartControls::valChanged);
  layout->addWidget (z_var_max, row, col++);
  
  z_label = new  QLineEdit ();
  z_label->setPlaceholderText ("z axix label");
  layout->addWidget (z_label, row, col++);
    

  /*  toggles */
  
  row++;
  col = 0;
  
  do_spline = new QCheckBox ("Spline");
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  &ChartControls::valChanged);
  
  do_polar = new QCheckBox ("Polar");
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  &ChartControls::valChanged);

  setLayout (layout);
}

void
ChartControls::valChanged (bool enabled __attribute__((unused)))
{
#if 0
  chartWindow->curve.polar  = (Qt::Checked == do_polar->checkState());
  chartWindow->curve.spline = (Qt::Checked == do_spline->checkState());
  //  curve.shorttitle
  chartWindow->curve.title			= chart_title->text ();
  //  chartWindow->curve.function.title		= y_label->text ();
  // chartWindow->curve.function.label		= fcn_label->text ();
  chartWindow->curve.function.expression	= apl_expression->text ();
  chartWindow->curve.ix.name			= x_var_name->text ();
  chartWindow->curve.ix.title			= x_label->text ();
  chartWindow->curve.ix.range.min		= x_var_min->value ();
  chartWindow->curve.ix.range.max		= x_var_max->value ();
  chartWindow->curve.iz.name			= z_var_name->text ();
  chartWindow->curve.iz.title			= z_label->text ();
  chartWindow->curve.iz.range.min 		= z_var_min->value ();
  chartWindow->curve.iz.range.max 		= z_var_max->value ();
  chartWindow->changed = true;
  chartWindow->handleExpression ();
#endif
}

void
ChartControls::valChangedv ()
{
  valChanged (true);
}

void
ChartControls::titleChangedv ()
{
  QString title = chart_title->text ();
  mainWindow->setTabTitle (tabIndex, title);
}

#if 0
void
ChartControls::enterChart (ChartWindow *cw)
{
  chart_title->setText (cw->curve.title);
  y_label->setText (cw->curve.function.label);

  x_var_name->setText (cw->curve.ix.name);
  x_var_min->setValue (cw->curve.ix.range.min);
  x_var_max->setValue (cw->curve.ix.range.max);
  x_label->setText (cw->curve.ix.title);

  z_var_name->setText (cw->curve.iz.name);
  z_var_min->setValue (cw->curve.iz.range.min);
  z_var_max->setValue (cw->curve.iz.range.max);
  z_label->setText (cw->curve.iz.title);

  fcn_label->setText (chartWindow->curve.function.title);
  apl_expression->setText (cw->curve.function.expression);

  do_spline->setCheckState (cw->curve.spline ? Qt::Checked : Qt::Unchecked);
  do_polar->setCheckState (cw->curve.polar   ? Qt::Checked : Qt::Unchecked);
  chartWindow = cw;
}
#endif

ChartControls::~ChartControls()
{
}
