
#include <QtWidgets>
#include <complex>

class
ComplexSpinBox : public QWidget
{
    Q_OBJECT

public:
  ComplexSpinBox ();
  ~ComplexSpinBox ();
  double getReal ();
  double getImag ();
  std::complex<double> getComplex ();
  void setReal (double rv);
  void setImag (double iv);
  void setComplex (double rv, double iv);
  void setComplex (std::complex<double> cv);

 Q_SIGNALS:
  void valueChanged ();

protected:

private slots:

private:
  void parseComplex (QString txt);

 QVBoxLayout *outer_layout;
 QValidator  *validator;
 QFrame      *frame;
 QHBoxLayout *layout;
 
 QGroupBox   *leftBox;
 QVBoxLayout *leftLayout;
 QToolButton *leftUp;
 QToolButton *leftDn;
 
 QGroupBox   *rightBox;
 QVBoxLayout *rightLayout;
 QToolButton *rightUp;
 QToolButton *rightDn;
 
 QLineEdit *ebox;
 double real;
 double imag;
};
