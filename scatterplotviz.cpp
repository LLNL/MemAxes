#include "scatterplotviz.h"

#include <QPaintEvent>

#include <cmath>
using namespace std;

#include "util.h"

ScatterPlotViz::ScatterPlotViz()
{
    VizWidget();

    // GLWidget options
    setMinimumSize(200, 200);
    setAutoFillBackground(false);

    // Set painting variables
    backgroundColor = QBrush(QColor(204, 229, 255));
    vizProcessed = false;

    xdim = 0;
    ydim = 0;
}

void ScatterPlotViz::processViz()
{
    points.clear();

    xMin = *(data->begin+xdim);
    xMax = *(data->begin+xdim);
    yMin = *(data->begin+ydim);
    yMax = *(data->begin+ydim);

    QVector<qreal>::Iterator p;
    qreal x,y;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        x = *(p+xdim);
        y = *(p+ydim);

        xMin = min(xMin,x);
        xMax = max(xMax,x);
        yMin = min(yMin,y);
        yMax = max(yMax,y);

        points.push_back(QPointF(x,y));
    }

    vizProcessed = true;
}

void ScatterPlotViz::setDims(int x, int y)
{
    vizProcessed = false;

    xdim = x;
    ydim = y;

    processViz();
    repaint();
}

void ScatterPlotViz::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{
    painter->fillRect(event->rect(), backgroundColor);

    if(!vizProcessed)
        return;

    int m = 20;
    plotBBox = QRectF(m,m,
                      event->rect().width()-m-m,
                      event->rect().height()-m-m);

    painter->setBrush(QBrush(QColor(0,0,0)));
    painter->drawLine(plotBBox.bottomLeft(),plotBBox.topLeft());
    painter->drawLine(plotBBox.bottomLeft(),plotBBox.bottomRight());

    painter->drawText(plotBBox.bottomLeft()+QPointF(0,10),data->meta[xdim]);
    painter->save();
    painter->translate(plotBBox.bottomLeft()-QPointF(5,0));
    painter->rotate(270);
    painter->drawText(QPointF(0,0),data->meta[ydim]);
    painter->restore();

    qreal xscale = plotBBox.width();
    qreal yscale = plotBBox.height();

    QPointF o = plotBBox.bottomLeft();
    QPointF p;

    // Draw unselected points
    painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for(int i=0; i<points.size(); i++)
    {
        if(data->selection[i] != 0)
            continue;

        p = QPointF(xscale*normalize(points[i].x(),xMin,xMax),
                    yscale*normalize(points[i].y(),yMin,yMax));
        p.setY(-p.y());

        painter->drawPoint(o+p);
    }

    // Draw selected points
    painter->setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for(int i=0; i<points.size(); i++)
    {
        if(data->selection[i] == 0)
            continue;

        p = QPointF(xscale*normalize(points[i].x(),xMin,xMax),
                    yscale*normalize(points[i].y(),yMin,yMax));
        p.setY(-p.y());

        if(data->selection[i] != 0)
        painter->drawPoint(o+p);
    }
}
