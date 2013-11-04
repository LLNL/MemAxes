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
