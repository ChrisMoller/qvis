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
#include "chartwindow.h"
#include "curves.h"

MainWindow *
ChartControls::getMainWindow ()
{
  return mainWindow;
}

ChartWindow *
ChartControls::getChartWindow ()
{
  return chartWindow;
}

void
ChartControls::curveSettings ()
{
  QDialog dialog (this, Qt::Dialog);
  QGridLayout *layout = new QGridLayout;
  dialog.setLayout (layout);

  QLabel lbl ("Theme");
  layout->addWidget (&lbl, 0, 0);
  
  QComboBox *themebox = new QComboBox ();
  themebox->addItem ("Light", QChart::ChartThemeLight);
  themebox->addItem ("Blue Cerulean", QChart::ChartThemeBlueCerulean);
  themebox->addItem ("Dark", QChart::ChartThemeDark);
  themebox->addItem ("Brown Sand", QChart::ChartThemeBrownSand);
  themebox->addItem ("Blue Ncs", QChart::ChartThemeBlueNcs);
  themebox->addItem ("High Contrast", QChart::ChartThemeHighContrast);
  themebox->addItem ("Blue Icy", QChart::ChartThemeBlueIcy);
  themebox->addItem ("Qt", QChart::ChartThemeQt);

#if 1
  chartData = this->getChartData ();
#else
#endif
  int sel = (int)chartData->getTheme ();
  themebox->setCurrentIndex (sel);
  
  layout->addWidget(themebox, 0, 1);
  QPushButton *cancelButton = new QPushButton (QObject::tr ("Close"));
  cancelButton->setAutoDefault (false);
  cancelButton->setDefault (false);
  layout->addWidget (cancelButton, 1, 0);
  QObject::connect (cancelButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);
  QPushButton *acceptButton = new QPushButton (QObject::tr ("Accept"));
  acceptButton->setAutoDefault (true);
  acceptButton->setDefault (true);
  layout->addWidget (acceptButton, 1, 1);
  QObject::connect (acceptButton, &QPushButton::clicked,
		    &dialog, &QDialog::accept);

  QPoint loc = mainWindow->pos ();
  dialog.move (loc.x () + 200, loc.y () + 200);
  if (QDialog::Accepted == dialog.exec ()) {
    theme = (QChart::ChartTheme)themebox->currentData ().toInt ();
    chartData->setTheme (theme);
  }
}

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

void
ChartControls::setUseState (bool state)
{
  useState = state;
}

bool
ChartControls::inUse ()
{
  return useState;
}

ChartData *
ChartControls::getChartData ()
{
  return chartData;
}

void
ChartControls::setChartData (ChartData *cd)
{
  chartData = cd;
}

ChartControls::ChartControls (int index, ChartData *cd, MainWindow *parent)
  : QWidget(parent)
{
  mainWindow = parent;
  tabIndex = index;
  chartData = cd;
  // /old_home/Qt/Examples/Qt-5.15.1/widgets/dialogs/tabdialog/tabdialog.cpp
  QGridLayout *layout = new QGridLayout ();
  theme = QChart::ChartThemeLight;
  useState = false;
  
  int row = 0;
  int col = 0;

  chart_title = new  QLineEdit ();
  chart_title->setPlaceholderText ("chart title");
  if (chartData) chart_title->setText (chartData->getTitle ());
  
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

  col++;

  QPushButton *settingsButton = new QPushButton (QObject::tr ("Settings"));
  layout->addWidget (settingsButton, row, col);
  QObject::connect (settingsButton, SIGNAL (clicked ()),
		    this, SLOT (curveSettings ()));
  
  row++;
  col = 0;

  /*  x indep vbl */

  Index *ix = nullptr;
  if (chartData) ix = chartData->getXIndex ();

  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  if (ix) x_var_name->setText (ix->getName ());
  layout->addWidget (x_var_name, row, col++);
  
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_min->setToolTip ("x minimum value");
  if (ix) x_var_min->setValue (ix->getMin ());
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    [=](double d)
		    { ix->setMin (d); mainWindow->notifyAll (); });
  layout->addWidget (x_var_min, row, col++);

  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  x_var_max->setToolTip ("x maximum value");
  if (ix) x_var_max->setValue (ix->getMax ());
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    { ix->setMax (d); mainWindow->notifyAll (); });
  layout->addWidget (x_var_max, row, col++);
  
  x_label = new  QLineEdit ();
  x_label->setPlaceholderText ("x axix label");
  if (ix) x_label->setText (ix->getLabel ());
  layout->addWidget (x_label, row, col++);

  /*  z indep vbl */

  row++;
  col = 0;
  
  Index *iz = nullptr;
  if (chartData) iz = chartData->getZIndex ();

  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  if (iz) z_var_name->setText (iz->getName ());
  layout->addWidget (z_var_name, row, col++);

  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_min->setToolTip ("z minimum value");
  if (iz) z_var_min->setValue (iz->getMin ());
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    { iz->setMin (d); mainWindow->notifyAll (); });
  layout->addWidget (z_var_min, row, col++);

  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  z_var_max->setToolTip ("z maximum value");
  if (iz) z_var_max->setValue (iz->getMax ());
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    { iz->setMax (d); mainWindow->notifyAll (); });
  layout->addWidget (z_var_max, row, col++);
  
  z_label = new  QLineEdit ();
  z_label->setPlaceholderText ("z axix label");
  if (iz) z_label->setText (iz->getLabel ());
  layout->addWidget (z_label, row, col++);
    

  /*  toggles */
  
  row++;
  col = 0;
  
  do_spline = new QCheckBox ("Spline");
  if (chartData && chartData->getSpline ())
    do_spline->setCheckState (Qt::Checked);
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  &ChartControls::valChanged);
  
  do_polar = new QCheckBox ("Polar");
  if (chartData && chartData->getPolar ())
    do_polar->setCheckState (Qt::Checked);
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  &ChartControls::valChanged);

  setLayout (layout);

  chartWindow = new ChartWindow (this);
  chartData->setWindow (chartWindow);
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
