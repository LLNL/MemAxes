#ifndef SCATTERPLOTVIZ_H
#define SCATTERPLOTVIZ_H

#include "vizwidget.h"

class ScatterPlotViz : public VizWidget
{
    Q_OBJECT
public:
    ScatterPlotViz();

public:
    void paint(QPainter *painter, QPaintEvent *event, int elapsed);
    void processViz();

signals:

public slots:
    void setDims(int x, int y);

private:
    long long xdim;
    long long ydim;

    qreal xMin;
    qreal xMax;
    qreal yMin;
    qreal yMax;

    QVector<QPointF> points;

    QRectF plotBBox;
};

#endif // SCATTERPLOTVIZ_H
