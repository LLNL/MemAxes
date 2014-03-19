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
    return nmin + (nmax-nmin) * ((val-omin) / (omax-omin));
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
        theta = theta;
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
                  QColor colorBarMin, QColor colorBarMax)
{
    if(val >= maxVal)
        return colorBarMax;
    if(val <= minVal)
        return colorBarMin;

    //qreal minH = colorBarMin.hueF();
    //qreal minS = colorBarMin.saturationF();
    //qreal minV = colorBarMin.valueF();

    //qreal maxH = colorBarMax.hueF();
    //qreal maxS = colorBarMax.saturationF();
    //qreal maxV = colorBarMax.valueF();

    qreal sv = normalize(val,minVal,maxVal);

    QColor result;
    qreal newH = lerp(sv,colorBarMin.redF(),colorBarMax.redF());//minH,maxH);
    qreal newS = lerp(sv,colorBarMin.greenF(),colorBarMax.greenF());//minS,maxS);
    qreal newV = lerp(sv,colorBarMin.blueF(),colorBarMax.blueF());//minV,maxV);

    result.setRgbF(newH,newS,newV);

    return result;
}
