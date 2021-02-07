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

#ifndef CHARTCONTROLS_H
#define CHARTCONTROLS_H

#include <QtWidgets>

#include "chartwindow.h"

class ChartControls : public QWidget
{
  Q_OBJECT

public:
  ChartControls (int index, MainWindow *parent);
  ~ChartControls ();
  QComboBox 	 *curves_combo;
  //  void 		 enterChart (ChartWindow *cw); 
  QLineEdit	 *chart_title;
  QCheckBox 	 *do_spline;
  QCheckBox 	 *do_polar;
  QLineEdit 	 *x_var_name;
  QLineEdit	 *x_label;
  QDoubleSpinBox *x_var_min;
  QDoubleSpinBox *x_var_max;
  QLineEdit 	 *z_var_name;
  QLineEdit 	 *z_label;
  QDoubleSpinBox *z_var_min;
  QDoubleSpinBox *z_var_max;
  QComboBox 	 *themebox;
  QList<int>      selected;
  QChart::ChartTheme theme;

 private:
  int		  tabIndex;
  MainWindow     *mainWindow;
  QVector<int>    curveList;
  ChartData 	 *chartData;			   

private slots:
  void valChanged(bool enabled);
  void valChangedv();
  void titleChangedv ();
  void selectCurves ();
  void curveSettings ();
};

#endif //  CHARTCONTROLS_H

