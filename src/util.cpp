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

#include "util.h"

qreal normalize(qreal val, qreal min, qreal max)
{
    return (val-min) / (max-min);
}

qreal lerp(qreal val, qreal min, qreal max)
{
    return min + (max-min)*val;
}

qreal scale(qreal val, qreal omin, qreal omax, qreal nmin, qreal nmax)
{
    qreal range = (omax-omin);
    range = (range == 0) ? 1 : range;
    return nmin + (nmax-nmin) * ((val-omin) / range);
}

qreal clamp(qreal val, qreal min, qreal max)
{
    if(val<min)
        return min;
    else if(val>max)
        return max;
    return val;
}

bool within(qreal val, qreal min, qreal max)
{
    return val >= min && val <= max;
}

bool overlap(qreal vmin, qreal vmax, qreal min, qreal max)
{
    return (within(vmin,min,max) || within(vmax,min,max) ||
            within(min,vmin,vmax) || within(max,vmin,vmax));
}

QPointF polarToCartesian(qreal mag, qreal theta)
{
    return polarToCartesian(QPointF(mag,theta));
}

QPointF polarToCartesian(QPointF point)
{
    float mag = point.x();
    float theta = point.y();

    float radAngle = -2*M_PI*((theta/16)/360);
    float xpos = mag * cos(radAngle);
    float ypos = mag * sin(radAngle);

    return QPointF(xpos,ypos);
}

#include <iostream>
using namespace std;
QPointF cartesianToPolar(QPointF point)
{
    QPointF xp(point.x(),-point.y());

    float mag = sqrt(xp.x()*xp.x() + xp.y()*xp.y());
    float theta = atan(xp.y() / xp.x());

    theta = 360*(theta/(2*M_PI));

    bool xpos = xp.x()>0;
    bool ypos = xp.y()>0;

    if(xpos && ypos)
        {/* do nothing */}
    else if(!xpos && !ypos)
        theta = 180+theta;
    else if(!xpos && ypos)
        theta = 180+theta;
    else //if(xpos && !ypos)
        theta = 360+theta;

    theta *= 16;

    return QPointF(mag,theta);
}

ColorMap gradientColorMap(QColor col0, QColor col1, int steps)
{
    ColorMap result;

    float deltaR = (float)(col1.red() - col0.red()) / (float)steps;
    float deltaG = (float)(col1.green() - col0.green()) / (float)steps;
    float deltaB = (float)(col1.blue() - col0.blue()) / (float)steps;
    for(float i=0; i<steps; i++)
    {
        QColor c(col0.red()+deltaR*i,
                 col0.green()+deltaG*i,
                 col0.blue()+deltaB*i);
        result.push_back(c);
    }
    return result;
}

QColor valToColor(qreal val, ColorMap colorMap)
{
    qreal sv = scale(val,0,1,0,colorMap.size());
    int colIdx = min(colorMap.size()-1,(int)floor(sv));

    return colorMap.at(colIdx);
}

QPointF radialTransform(QPointF point, QRectF rectSpace)
{
    // Get radius
    float radius = min(rectSpace.width(),rectSpace.height()) / 2.0f;

    // Transform to square [radius,radius]
    point.setX(scale(point.x(),rectSpace.left(),rectSpace.right(),0,360*16));
    point.setY(scale(point.y(),rectSpace.top(),rectSpace.bottom(),0,radius));

    // Y value = magnitude, X value = angle
    float mag = point.y();
    float theta = point.x();

    point = polarToCartesian(mag,theta);

    // Back into box space
    point += QPointF(rectSpace.width()/2,rectSpace.height()/2);
    point += QPointF(rectSpace.left(),rectSpace.top());

    return point;
}

QPointF reverseRadialTransform(QPointF point, QRectF rectSpace)
{
    // Get radius
    float radius = min(rectSpace.width(),rectSpace.height()) / 2.0f;

    point -= QPointF(rectSpace.width()/2,rectSpace.height()/2);
    point -= QPointF(rectSpace.left(),rectSpace.top());

    point = cartesianToPolar(point);

    float mag = point.x();
    float theta = point.y();

    point.setX(scale(theta,0,360*16,rectSpace.left(),rectSpace.right()));
    point.setY(scale(mag,0,radius,rectSpace.top(),rectSpace.bottom()));

    return point;
}

QVector<QPointF> rectToRadialSegment(QRectF rect, QRectF rectSpace)
{
    QVector<QPointF> segmentPoly;

    int xres = scale(rect.width(),0,rectSpace.width(),1,40);

    QPointF p = rect.topLeft();
    QPointF q = rect.bottomRight();

    float dx = (q.x() - p.x()) / (float)xres;
    for(int i=0; i<=xres; i++)
    {
        QPointF rp = radialTransform(p,rectSpace);
        segmentPoly.push_back(rp);
        p.setX(p.x()+dx);
    }

    for(int i=0; i<=xres; i++)
    {
        QPointF rq = radialTransform(q,rectSpace);
        segmentPoly.push_back(rq);
        q.setX(q.x()-dx);
    }

    return segmentPoly;
}
