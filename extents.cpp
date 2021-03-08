
#include <QtWidgets>
#include <values.h>
#include "extents.h"

Extents::Extents ()
{
  fprintf (stderr, "extents cons\n");
  x_max = -MAXDOUBLE;
  x_min =  MAXDOUBLE;
  y_max = -MAXDOUBLE;
  y_min =  MAXDOUBLE;
  z_max = -MAXDOUBLE;
  z_min =  MAXDOUBLE;
}

void
Extents::clear ()
{
  fprintf (stderr, "extents clr\n");
  x_max = -MAXDOUBLE;
  x_min =  MAXDOUBLE;
  y_max = -MAXDOUBLE;
  y_min =  MAXDOUBLE;
  z_max = -MAXDOUBLE;
  z_min =  MAXDOUBLE;
}

void
Extents::setX (double x)
{
  if (x_max < (qreal)x) x_max = (qreal)x;
  if (x_min > (qreal)x) x_min = (qreal)x;
}

void
Extents::setY (double y)
{
  if (y_max < (qreal)y) y_max = (qreal)y;
  if (y_min > (qreal)y) y_min = (qreal)y;
}

void
Extents::setZ (double z)
{
  if (z_max < (qreal)z) z_max = (qreal)z;
  if (z_min > (qreal)z) z_min = (qreal)z;
}

void
Extents::setX (double min, double max)
{
  x_min = min;
  x_max = max;
}

void
Extents::setY (double min, double max)
{
  y_min = min;
  y_max = max;
}

void
Extents::setZ (double min, double max)
{
  z_min = min;
  z_max = max;
}

double
Extents::deltaX ()
{
  return x_max - x_min;
}

double
Extents::deltaY ()
{
  return y_max - y_min;
}
double
Extents::deltaZ ()
{
  return z_max - z_min;
}

double
Extents::sumX ()
{
  return x_max + x_min;
}

double
Extents::sumY ()
{
  return y_max + y_min;
}

double
Extents::sumZ ()
{
  return z_max + z_min;
}

double
Extents::minX ()
{
  return x_min;
}

double
Extents::maxX ()
{
  return x_max;
}

double
Extents::minY ()
{
  return y_min;
}

double
Extents::maxY ()
{
  return y_max;
}

double
Extents::minZ ()
{
  return z_min;
}

double
Extents::maxZ ()
{
  return z_max;
}
