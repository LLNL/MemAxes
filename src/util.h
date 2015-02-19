//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <QVector>
#include <QColor>
#include <iostream>

#define COLMAJOR_2D(x,y,d) x*d+y
#define ROWMAJOR_2D(x,y,d) y*d+x
#define DBGLN(x) std::cerr << #x << std::endl; x;
#define DBGVAR(x) std::cerr << #x << " : " << x << std::endl;

typedef QVector<QColor> ColorMap;
typedef QPair<int,int> IntRange;
typedef QPair<qreal,qreal> RealRange;

qreal normalize(qreal val, qreal min, qreal max);
qreal scale(qreal val, qreal omin, qreal omax, qreal nmin, qreal nmax);
qreal lerp(qreal val, qreal min, qreal max);
qreal clamp(qreal val, qreal min, qreal max);
bool within(qreal val, qreal min, qreal max);
bool overlap(qreal vmin, qreal vmax, qreal min, qreal max);

QPointF polarToCartesian(qreal mag, qreal theta);
QPointF polarToCartesian(QPointF point);
QPointF cartesianToPolar(QPointF point);

QPointF radialTransform(QPointF point, QRectF rectSpace);
QPointF reverseRadialTransform(QPointF point, QRectF rectSpace);
QVector<QPointF> rectToRadialSegment(QRectF rect, QRectF rectSpace);

ColorMap gradientColorMap(QColor col0, QColor col1, int steps);
QColor valToColor(qreal val, ColorMap colorMap);

#endif // UTIL_H
