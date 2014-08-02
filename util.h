#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <QVector>
#include <QColor>
#include <iostream>

#define COLMAJOR_2D(x,y,d) x*d+y
#define ROWMAJOR_2D(x,y,d) y*d+x
#define DBGLN(x) std::cout << #x << std::endl; x
#define DBGVAR(x) std::cout << #x << " : " << (x) << std::endl;

typedef QVector<QColor> ColorMap;
typedef QPair<int,int> IntRange;
typedef QPair<qreal,qreal> RealRange;

qreal normalize(qreal val, qreal min, qreal max);
qreal scale(qreal val, qreal omin, qreal omax, qreal nmin, qreal nmax);
qreal lerp(qreal val, qreal min, qreal max);
qreal clamp(qreal val, qreal min, qreal max);
bool within(qreal val, qreal min, qreal max);

QPointF polarToCartesian(qreal mag, qreal theta);
QPointF polarToCartesian(QPointF point);
QPointF cartesianToPolar(QPointF point);

QPointF radialTransform(QPointF point, QRectF rectSpace);
QPointF reverseRadialTransform(QPointF point, QRectF rectSpace);
QVector<QPointF> rectToRadialSegment(QRectF rect, QRectF rectSpace);

QColor valToColor(qreal val, qreal minVal, qreal maxVal,
                  ColorMap colorMap);

#endif // UTIL_H
