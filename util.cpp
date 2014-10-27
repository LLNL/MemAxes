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

QColor valToColor(qreal val, qreal minVal, qreal maxVal,
                  ColorMap colorMap)
{
    qreal sv = scale(val,minVal,maxVal,0,colorMap.size());
    int colIdx = min(colorMap.size()-1,(int)floor(sv));

    return colorMap[colIdx];
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
