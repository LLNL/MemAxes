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
