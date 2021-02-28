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
  
  chartData = this->getChartData ();

  int row = 0;

#if 0
  /********* surface mode ********/
  
  QLabel lblm ("Surface mode");
  layout->addWidget (&lblm, row, 0);
  
  QComboBox *smodebox = new QComboBox ();
  smodebox->addItem ("Surface",    QSurface3DSeries::DrawSurface);
  smodebox->addItem ("Wire Frame", QSurface3DSeries::DrawWireframe);
  smodebox->addItem ("Suface + Wire Frame",
		     QSurface3DSeries::DrawSurfaceAndWireframe);
  QSurface3DSeries::DrawFlags selm = chartData->getDrawMode ();
  int loc = smodebox->findData (QVariant (selm));
  smodebox->setCurrentIndex (loc);

  layout->addWidget (smodebox, row, 1);

  row++;

#endif

  /******* theme ******/
  
  QLabel lbl ("Theme");
  layout->addWidget (&lbl, row, 0);
  
  QComboBox *themebox = new QComboBox ();
  themebox->addItem ("Light", QChart::ChartThemeLight);
  themebox->addItem ("Blue Cerulean", QChart::ChartThemeBlueCerulean);
  themebox->addItem ("Dark", QChart::ChartThemeDark);
  themebox->addItem ("Brown Sand", QChart::ChartThemeBrownSand);
  themebox->addItem ("Blue Ncs", QChart::ChartThemeBlueNcs);
  themebox->addItem ("High Contrast", QChart::ChartThemeHighContrast);
  themebox->addItem ("Blue Icy", QChart::ChartThemeBlueIcy);
  themebox->addItem ("Qt", QChart::ChartThemeQt);
  int sel = (int)chartData->getTheme ();
  themebox->setCurrentIndex (sel);
  connect (themebox, QOverload<int>::of(&QComboBox::activated),
	  [=](int index)
	  {QVariant sel = themebox->itemData (index);
	    chartData->setTheme (sel.toInt ());
	    mainWindow->updateAll ();
	    mainWindow->notifySelective (true); });
  layout->addWidget(themebox, row, 1);

  row++;


  /****** background image  ******/

  
  QLabel lblb ("Background");
  layout->addWidget (&lblb, row, 0);

  QFileInfo fi (chartData->getBGFile ());
  QPushButton *imgButton = new QPushButton (fi.baseName ());
  layout->addWidget(imgButton, row, 1);
  connect (imgButton,
	   &QAbstractButton::clicked,
	   [=](){ 
	     QString fileName =
	       QFileDialog::getOpenFileName (this,
				     tr("Open Image"),
				     ".",
			     tr("Image Files (*.png *.jpg *.bmp)"));
	     chartData->setBGFile (fileName);
	     QFileInfo fi (fileName);
	     imgButton->setText (fi.baseName ());
	     chartData->setUpdate (true); // mainWindow->updateAll ();
	     mainWindow->notifySelective (true);
	   });

  QPushButton *clearButton = new QPushButton ("Clear");
  layout->addWidget(clearButton, row, 2);
  connect (clearButton,
	   &QAbstractButton::clicked,
	   [=](){ chartData->clearBGFile ();
	     clearButton->setText ("");
	     chartData->setUpdate (true); // mainWindow->updateAll ();
	     mainWindow->notifySelective (true);
	   });

  row++;

  QPushButton *cancelButton = new QPushButton (QObject::tr ("Close"));
  cancelButton->setAutoDefault (false);
  cancelButton->setDefault (false);
  layout->addWidget (cancelButton, row, 1);
  QObject::connect (cancelButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);

  bool run = true;
  while (run) {
    QPoint loc = mainWindow->pos ();
    dialog.move (loc.x () + 200, loc.y () + 200);
    if (QDialog::Rejected == dialog.exec ()) run = false;
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
    bool active;
    if (chartData)
      active = chartData->getSelected ().contains (i);
    else active = false;

    QCheckBox *label_check =
      new QCheckBox (mainWindow->getCurve (i).getLabel ());
    label_check->setCheckState (active ? Qt::Checked : Qt::Unchecked);
    connect (label_check, QOverload<int>::of(&QCheckBox::stateChanged),
	     [=](int state)
	     {chartData->setSelected (i, state);
	       chartData->setUpdate (true);
	       mainWindow->notifySelective (true); });
    curvesTable->setCellWidget (i, 0, label_check);
  }

  dialog_layout->addWidget (curvesTable);

  QPushButton *closeButton = new QPushButton (QObject::tr ("Close"));
  dialog_layout->addWidget (closeButton);
  QObject::connect (closeButton, &QPushButton::clicked,
		    &dialog, &QDialog::reject);

  bool run = true;
  while (run) {
    QPoint loc = mainWindow->pos ();
    dialog.move (loc.x () + 200, loc.y () + 200);
    if (QDialog::Rejected == dialog.exec ()) run = false;
  }
#if 0
  if (QDialog::Accepted == dialog.exec ()) {
    selected.clear ();
    for (i = 0; i < mainWindow->getCurveCount (); i++) {
      QTableWidgetItem *item =curvesTable->item (i, 0);
      Qt::CheckState cs = item->checkState ();
      if (cs == Qt::Checked) selected.append (i);
    }
  }
#endif
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

  QPushButton *curvesButton =
    new QPushButton (QObject::tr ("Select curves"));
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

  //		 QOverload<const QString &text>::of(&QLineEdit::textChanged),
  //		    &QLineEdit::textChanged,
  x_var_name = new  QLineEdit ();
  x_var_name->setPlaceholderText ("x variable name");
  if (ix) x_var_name->setText (ix->getName ());
  layout->addWidget (x_var_name, row, col++);
  QObject::connect (x_var_name,
		    &QLineEdit::returnPressed,
		    [=]()
		    {Index *ix = chartData->getXIndex ();
		      ix->setName (x_var_name->text ());
		      chartData->setUpdate (true);
		      mainWindow->notifySelective (false); });
  
  x_var_min = new  QDoubleSpinBox ();
  x_var_min->setToolTip ("Increment/decrement X<sub>min</sub>.  Hold to repeat.");
  x_var_min->setAccelerated (true);
  x_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  if (ix) x_var_min->setValue (ix->getMin ());
  QObject::connect (x_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    [=](double d)
		    { chartData->setUpdate (true);
		      ix->setMin (d); mainWindow->notifySelective (false); });
  layout->addWidget (x_var_min, row, col++);

  x_var_max = new  QDoubleSpinBox ();
  x_var_max->setToolTip ("Increment/decrement X<sub>max</sub>.  Hold to repeat.");
  x_var_max->setAccelerated (true);
  x_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  if (ix) x_var_max->setValue (ix->getMax ());
  QObject::connect (x_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    {
		      if (!ix) return;
		      chartData->setUpdate (true);
		      ix->setMax (d);
		      mainWindow->notifySelective (false);
		    });
  layout->addWidget (x_var_max, row, col++);
  
  x_label = new  QLineEdit ();
  x_label->setPlaceholderText ("x axix label");
  if (ix) x_label->setText (ix->getLabel ());
  layout->addWidget (x_label, row, col++);
  QObject::connect (x_label,
		    &QLineEdit::returnPressed,
		    [=]()
		    {
		      if (!ix) return;
		      Index *ix = chartData->getXIndex ();
		      ix->setLabel (x_label->text ());
		      chartData->setUpdate (true);
		      mainWindow->notifySelective (false);
		    });

  /*  z indep vbl */

  row++;
  col = 0;
  
  Index *iz = nullptr;
  if (chartData) iz = chartData->getZIndex ();

  z_var_name = new  QLineEdit ();
  z_var_name->setPlaceholderText ("z variable name");
  if (iz) z_var_name->setText (iz->getName ());
  layout->addWidget (z_var_name, row, col++);
  QObject::connect (z_var_name,
		    &QLineEdit::returnPressed,
		    [=]()
		    {
		      if (!iz) return;
		      Index *iz = chartData->getXIndex ();
		      iz->setName (z_var_name->text ());
		      chartData->setUpdate (true);
		      mainWindow->notifySelective (false);
		    });

  z_var_min = new  QDoubleSpinBox ();
  z_var_min->setToolTip ("Increment/decrement Z<sub>min</sub>.  Hold to repeat.");
  z_var_min->setAccelerated (true);
  z_var_min->setRange (-MAXDOUBLE, MAXDOUBLE);
  if (iz) z_var_min->setValue (iz->getMin ());
  QObject::connect (z_var_min,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    {
		      if (!iz) return;
		      chartData->setUpdate (true);
		      iz->setMin (d);
		      mainWindow->notifySelective (false);
		    });
  layout->addWidget (z_var_min, row, col++);

  z_var_max = new  QDoubleSpinBox ();
  z_var_max->setToolTip ("Increment/decrement Z<sub>max</sub>.  Hold to repeat.");
  z_var_max->setAccelerated (true);
  z_var_max->setRange (-MAXDOUBLE, MAXDOUBLE);
  if (iz) z_var_max->setValue (iz->getMax ());
  QObject::connect (z_var_max,
		    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		    this,
		    [=](double d )
		    { chartData->setUpdate (true);
		      iz->setMax (d); mainWindow->notifySelective (false); });
  layout->addWidget (z_var_max, row, col++);
  
  z_label = new  QLineEdit ();
  z_label->setPlaceholderText ("z axix label");
  if (iz) z_label->setText (iz->getLabel ());
  layout->addWidget (z_label, row, col++);
  QObject::connect (z_label,
		    &QLineEdit::returnPressed,
		    [=]()
		    {Index *iz = chartData->getXIndex ();
		      iz->setLabel (z_label->text ());
		      chartData->setUpdate (true);
		      mainWindow->notifySelective (false); });
    

  /*  toggles */
  
  row++;
  col = 0;

#if 0
  do_spline = new QCheckBox ("Spline");
  if (chartData && chartData->getSpline ())
    do_spline->setCheckState (Qt::Checked);
  layout->addWidget (do_spline, row, col++);
  connect(do_spline,
	  &QCheckBox::stateChanged,
	  this,
	  [=]()
	  {chartData->setUpdate (true);
	   mainWindow->notifySelective (false); });
#endif
  
  do_polar = new QCheckBox ("Polar");
  if (chartData && chartData->getPolar ())
    do_polar->setCheckState (Qt::Checked);
  layout->addWidget (do_polar, row, col++);
  connect(do_polar,
	  &QCheckBox::stateChanged,
	  this,
	  [=]()
	  {chartData->setUpdate (true);
	   mainWindow->notifySelective (false); });

#if 1
  QDoubleSpinBox *incrBox = new QDoubleSpinBox ();
  incrBox->setDecimals (0);
  incrBox->setRange (16.0, 128.0);
  incrBox->setValue ( (double)mainWindow->getIncr ());
  layout->addWidget (incrBox, row, col++);
  connect (incrBox,
	  QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	  this,
	  [=](double val)
	  {
	    mainWindow->setIncr ((int)val);
	    chartData->setUpdate (true);
	    mainWindow->notifySelective (false);
	  });
#else					// buttons dont show up
  QSpinBox *incrBox = new QSpinBox ();
#if 0
  incrBox->setStyleSheet("QSpinBox::up-arrow {  width: 7px;  height: 7px; }"
                     "QSpinBox::down-arrow {  width: 10px;  height: 7px; }");
  incrBox->setButtonSymbols (QAbstractSpinBox::UpDownArrows);
#endif
  incrBox->setRange (16, 128);
  incrBox->setValue ( mainWindow->getIncr ());
  layout->addWidget (incrBox, row, col++);
  connect (incrBox,
	  QOverload<int>::of(&QSpinBox::valueChanged),
	  this,
	  [=](int val)
	  {
	    mainWindow->setIncr (val);
	    chartData->setUpdate (true);
	    mainWindow->notifySelective (false);
	  });
#endif

  setLayout (layout);

  chartWindow = new ChartWindow (this);
  if (chartData) chartData->setWindow (chartWindow);
}

ChartControls::~ChartControls()
{
}
