#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

#define COLMAJOR_2D(x,y,d) x*d+y
#define ROWMAJOR_2D(x,y,d) y*d+x

qreal normalize(qreal val, qreal min, qreal max);
qreal scale(qreal val, qreal omin, qreal omax, qreal nmin, qreal nmax);
qreal lerp(qreal val, qreal min, qreal max);
qreal clamp(qreal val, qreal min, qreal max);

#endif // UTIL_H
