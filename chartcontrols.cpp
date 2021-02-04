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

//QT_CHARTS_USE_NAMESPACE

ChartControls::ChartControls (QWidget *parent)
  : QWidget(parent)
{
  // /old_home/Qt/Examples/Qt-5.15.1/widgets/dialogs/tabdialog/tabdialog.cpp
#if 0
  QVBoxLayout *outer = new QVBoxLayout ();
  QGroupBox *formGroupBox = new QGroupBox ("Controls");
  outer->addWidget (formGroupBox);
#endif
  QGridLayout *layout = new QGridLayout ();
  
  int row = 0;
  int col = 0;

  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  layout->addWidget (chart_title, row, 0, 1, 3);

  y_title = new  QLineEdit ();
  y_title->setPlaceholderText ("y axix label");
  layout->addWidget (y_title, row, 3);


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
  
  x_title = new  QLineEdit ();
  x_title->setPlaceholderText ("x axix label");
  layout->addWidget (x_title, row, col++);

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
  
  z_title = new  QLineEdit ();
  z_title->setPlaceholderText ("z axix label");
  layout->addWidget (z_title, row, col++);
    
  /*  APL expression */

  row++;
  col = 0;
    
  fcn_label = new  QLineEdit ();
  fcn_label->setPlaceholderText ("curve label");
  layout->addWidget (fcn_label, row, col++);

  apl_expression = new  QLineEdit ();
  apl_expression->setPlaceholderText ("function");
  layout->addWidget (apl_expression, row, col, 1, 3);
  QObject::connect (apl_expression,
		    &QLineEdit::editingFinished,
		    this,
		    &ChartControls::valChangedv);

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

  //  formGroupBox->show ();
  //  return formGroupBox;
#if 0
  setLayout (outer);
#else
  setLayout (layout);
#endif
}

void
ChartControls::valChanged (bool enabled __attribute__((unused)))
{
  chartWindow->curve.polar  = (Qt::Checked == do_polar->checkState());
  chartWindow->curve.spline = (Qt::Checked == do_spline->checkState());
  //  curve.shorttitle
  chartWindow->curve.title			= chart_title->text ();
  chartWindow->curve.function.title		= y_title->text ();
  chartWindow->curve.function.label		= fcn_label->text ();
  chartWindow->curve.function.expression	= apl_expression->text ();
  chartWindow->curve.ix.name			= x_var_name->text ();
  chartWindow->curve.ix.title			= x_title->text ();
  chartWindow->curve.ix.range.min		= x_var_min->value ();
  chartWindow->curve.ix.range.max		= x_var_max->value ();
  chartWindow->curve.iz.name			= z_var_name->text ();
  chartWindow->curve.iz.title			= z_title->text ();
  chartWindow->curve.iz.range.min 		= z_var_min->value ();
  chartWindow->curve.iz.range.max 		= z_var_max->value ();
  chartWindow->changed = true;
  chartWindow->handleExpression ();
}

void
ChartControls::valChangedv ()
{
  valChanged (true);
}

#if 0
void
ChartControls::enterChart (ChartWindow *cw)
{
  chart_title->setText (cw->curve.title);
  y_title->setText (cw->curve.function.label);

  x_var_name->setText (cw->curve.ix.name);
  x_var_min->setValue (cw->curve.ix.range.min);
  x_var_max->setValue (cw->curve.ix.range.max);
  x_title->setText (cw->curve.ix.title);

  z_var_name->setText (cw->curve.iz.name);
  z_var_min->setValue (cw->curve.iz.range.min);
  z_var_max->setValue (cw->curve.iz.range.max);
  z_title->setText (cw->curve.iz.title);

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
