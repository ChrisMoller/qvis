
#include <QtWidgets>

#include "complexspinbox.h"

#define RE_FP "([-+]?([0-9]+(\\.[0-9]+)?|\\.[0-9]+))([eE]([-+]?[0-9]+))?"

/***
list[0] = 7.6e9j77		// whole thing
list[1] = 7.6e9			// real
list[2] = 7.6			// real mant
list[3] = 7.6			// real mant
list[4] = .6
list[5] = e9
list[6] = 9			// real exp
list[7] = 
list[8] =  			// imag
list[9] =			// imag mant
list[10] = 			// imag mant
list[11] = 			
list[12] = 			
list[13] =			//imag exp
 ***/

#ifndef toCString
#define toCString(v) ((v).toStdString ().c_str ())
#endif

static QString cpxval = QString ("(%1)([jJ](%1))?").arg (RE_FP);
static QRegExp rx(cpxval);

void
ComplexSpinBox::parseComplex (QString txt)
{
  bool rc = rx.exactMatch (txt);
  if (rc) {
    QStringList list = rx.capturedTexts();
    real = list[1].toFloat ();
    imag = list[8].toFloat ();
  }
}

double
ComplexSpinBox::getReal ()
{
  return real;
}

double
ComplexSpinBox::getImag ()
{
  return imag;
}


std::complex<double>
ComplexSpinBox::getComplex ()
{
  std::complex<double> val (real, imag);
  return val;
}

void
ComplexSpinBox::setReal (double rv)
{
  real = rv;
  QString txt = QString ("%1l%2").arg (real).arg (imag);
  ebox->setText (txt);
}

void
ComplexSpinBox::setImag (double iv)
{
  imag = iv;
  QString txt = QString ("%1l%2").arg (real).arg (imag);
  ebox->setText (txt);
}

void
ComplexSpinBox::setComplex (double rv, double iv)
{
  real = rv;
  imag = iv;
  QString txt = QString ("%1l%2").arg (real).arg (imag);
  ebox->setText (txt);
}

void
ComplexSpinBox::setComplex (std::complex<double> cv)
{
  real = cv.real ();
  imag = cv.imag ();
  QString txt = QString ("%1l%2").arg (real).arg (imag);
  ebox->setText (txt);
}

ComplexSpinBox::ComplexSpinBox ()
{
  real = 0.0;
  imag = 0.0;
  
  validator = new QRegExpValidator(rx, this);

  QFrame *frame = new QFrame ();
  //  frame->setStyleSheet ("margin:1px; border: 1px; padding: 1ps");
  frame->setLineWidth (2);
  frame->setFrameStyle(QFrame::Box | QFrame::Raised);

  outer_layout = new QVBoxLayout ();
  outer_layout->addWidget (frame);
  outer_layout->setContentsMargins (0,0,0,0);
  
  layout = new QHBoxLayout ();
  layout->setSpacing (0);
  layout->setMargin (0);
  
  leftBox = new QGroupBox ();
  leftBox->setStyleSheet ("margin:0px; border: 0px; padding: 0ps");
  leftLayout = new QVBoxLayout ();
  leftLayout->setSpacing (0);
  leftLayout->setMargin (0);
  leftLayout->setContentsMargins (0,0,0,0);
  
  leftUp = new QToolButton ();
  leftUp->setAutoRepeat (true);
  leftUp->setArrowType (Qt::UpArrow);
  leftUp->setToolTip ("Increment real component.  Hold to repeat.");
  //leftUp->setMaximumSize (22, 22);
  connect (leftUp, &QToolButton::clicked, this,
	   [=](){
	     real += 1.0;
	     QString txt = QString ("%1j%2").arg (real).arg (imag);
	     ebox->setText (txt);
	     Q_EMIT valueChanged ();
	   });
  leftLayout->addWidget (leftUp);

  leftDn = new QToolButton ();
  leftDn->setAutoRepeat (true);
  leftDn->setArrowType (Qt::DownArrow);
  leftDn->setToolTip ("Decrement real component.  Hold to repeat.");
  //leftDn->setMaximumSize (22, 22);
  connect (leftDn, &QToolButton::clicked, this,
	   [=](){
	     real -= 1.0;
	     QString txt = QString ("%1j%2").arg (real).arg (imag);
	     ebox->setText (txt);
	     Q_EMIT valueChanged ();
	   });
  leftLayout->addWidget (leftDn);

  leftBox->setLayout (leftLayout);
  leftBox->setMaximumSize (22, 48);
  layout->addWidget (leftBox);

  ebox = new QLineEdit ("0.0j0.0");
  ebox->setValidator(validator);
  layout->addWidget (ebox);
  connect (ebox,
	   &QLineEdit::returnPressed,
	   [=](){
	     parseComplex (ebox->text ());
	     Q_EMIT valueChanged ();
	   });

  rightBox = new QGroupBox ();
  rightBox->setStyleSheet ("margin:0px; border: 0px; padding: 0ps");
  rightLayout = new QVBoxLayout ();
  rightLayout->setSpacing (0);
  rightLayout->setMargin (0);
  
  rightUp = new QToolButton ();
  rightUp->setAutoRepeat (true);
  rightUp->setArrowType (Qt::UpArrow);
  rightUp->setToolTip ("Increment imaginary component.  Hold to repeat.");
  //rightUp->setMaximumSize (16, 16);
  connect (rightUp, &QToolButton::clicked, this,
	   [=](){
	     imag += 1.0;
	     QString txt = QString ("%1j%2").arg (real).arg (imag);
	     ebox->setText (txt);
	     Q_EMIT valueChanged ();
	   });
  rightLayout->addWidget (rightUp);

  rightDn = new QToolButton ();
  rightDn->setAutoRepeat (true);
  rightDn->setArrowType (Qt::DownArrow);
  rightDn->setToolTip ("Decrement imaginary component.  Hold to repeat.");
  //rightDn->setMaximumSize (16, 16);
  connect (rightDn, &QToolButton::clicked, this,
	   [=](){
	     imag -= 1.0;
	     QString txt = QString ("%1j%2").arg (real).arg (imag);
	     ebox->setText (txt);
	     Q_EMIT valueChanged ();
	   });
  rightLayout->addWidget (rightDn);

  rightBox->setLayout (rightLayout);
  rightBox->setMaximumSize (22, 48);
  layout->addWidget (rightBox);

  frame->setLayout (layout);
  this->setLayout (outer_layout);
}

ComplexSpinBox::~ComplexSpinBox ()
{
  delete ebox;
  delete validator;
  delete outer_layout;
  delete layout;
  delete leftBox;
  delete rightBox;
}

