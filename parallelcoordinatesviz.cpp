#include "parallelcoordinatesviz.h"

#include <QPaintEvent>

#include <iostream>
#include <cmath>
using namespace std;

#include "util.h"

ParallelCoordinatesViz::ParallelCoordinatesViz() :
    VizWidget()
{
    cursorPos.setX(-1);
    selecting = -1;

    selOpacity = 0.1;
    unselOpacity = 0.01;

    // Event Filters
    this->installEventFilter(this);
    this->setMouseTracking(true);

}

void ParallelCoordinatesViz::processViz()
{
    dimMins.resize(data->numDimensions);
    dimMaxes.resize(data->numDimensions);

    selMins.resize(data->numDimensions);
    selMaxes.resize(data->numDimensions);

    selMins.fill(-1);
    selMaxes.fill(-1);

    axesPositions.resize(data->numDimensions);
    axesOrder.resize(data->numDimensions);

    pointLines.resize(data->numElements*(data->numDimensions-1));

    for(int i=0; i<data->numDimensions; i++)
    {
        dimMins[i] = *(data->begin+i);
        dimMaxes[i] = *(data->begin+i);

        axesPositions[i] = i*(1.0/(data->numDimensions-1));
    }

    QVector<qreal>::Iterator p;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        for(int i=0; i<data->numDimensions; i++)
        {
            dimMins[i] = min(dimMins[i],*(p+i));
            dimMaxes[i] = max(dimMaxes[i],*(p+i));
        }
    }

    vizProcessed = true;
}

void ParallelCoordinatesViz::setSelOpacity(int val)
{
    selOpacity = (qreal)val/1000.0;
    repaint();
}

void ParallelCoordinatesViz::setUnselOpacity(int val)
{
    unselOpacity = (qreal)val/1000.0;
    repaint();
}

void ParallelCoordinatesViz::leaveEvent(QEvent *e)
{
    repaint();
}

void ParallelCoordinatesViz::mousePressEvent(QMouseEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    if(cursorPos.x() != -1)
    {
        selecting = cursorPos.x();
        firstSel = mouseEvent->pos().y();
    }
}

void ParallelCoordinatesViz::mouseReleaseEvent(QMouseEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    if(lastSel == -1)
    {
        if(selecting != -1)
        {
            selMins[selecting] = -1;
            processSelection();
        }

        repaint();
    }

    selecting = -1;
    lastSel = -1;
}

bool ParallelCoordinatesViz::eventFilter(QObject *obj, QEvent *event)
{
    if(!vizProcessed)
        return false;

    static qreal prevCursor = cursorPos.x();

    qreal axisMargin = 10;
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Change selection
        if(selecting != -1)
        {
            lastSel = clamp((qreal)mouseEvent->pos().y(),plotBBox.top(),plotBBox.bottom());

            qreal selmin = min(firstSel,lastSel);
            qreal selmax = max(firstSel,lastSel);

            // scale/invert
            selMins[selecting] = 1.0-scale(selmax,plotBBox.top(),plotBBox.bottom(),0,1);
            selMaxes[selecting] = 1.0-scale(selmin,plotBBox.top(),plotBBox.bottom(),0,1);

            processSelection();
        }

        // Get cursor location
        cursorPos.setX(-1);
        for(int i=0; i<data->numDimensions; i++)
        {
            qreal left = plotBBox.left()+axesPositions[i]*plotBBox.width()-axisMargin;
            qreal right = left+2.0*axisMargin;
            if(left < mouseEvent->pos().x()
                    && mouseEvent->pos().x() <= right
                    && plotBBox.top() < mouseEvent->pos().y()
                    && mouseEvent->pos().y() <= plotBBox.bottom())
            {
                cursorPos = QPointF(i,mouseEvent->pos().y());
                break;
            }
        }

        if(cursorPos.x() != -1 || prevCursor != cursorPos.x())
            repaint();

        prevCursor = cursorPos.x();
    }

    return false;
}

void ParallelCoordinatesViz::processSelection()
{
    QVector<qreal> dataSelMins(selMins);
    QVector<qreal> dataSelMaxes(selMaxes);

    int selAxes = 0;
    for(int i=0; i<dataSelMins.size(); i++)
    {
        if(selMins[i] != -1)
        {
            dataSelMins[i] = lerp(dataSelMins[i],dimMins[i],dimMaxes[i]);
            dataSelMaxes[i] = lerp(dataSelMaxes[i],dimMins[i],dimMaxes[i]);

            selAxes++;
        }
    }

    if(selAxes == 0)
    {
        data->selection.fill(0);
        emit repaintAll();
        return;
    }

    int elem = 0;
    QVector<qreal>::Iterator p;
    int select;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        select = 0;
        for(int i=0; i<data->numDimensions; i++)
        {
            if(selMins[i] != -1
               && dataSelMins[i] <= *(p+i)
               && *(p+i) <= dataSelMaxes[i])
            {
                select++;
            }
        }
        data->selection[elem] = (select == selAxes) ? 1 : 0;
        elem++;
    }

    emit repaintAll();
}

void ParallelCoordinatesViz::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{
    painter->fillRect(event->rect(), backgroundColor);

    if(!vizProcessed)
        return;

    int mx=40;
    int my=30;

    plotBBox = QRectF(mx,my,
                      event->rect().width()-mx-mx,
                      event->rect().height()-my-my);

    // Draw axes
    QPointF a = plotBBox.bottomLeft();
    QPointF b = plotBBox.topLeft();

    QFontMetrics fm = painter->fontMetrics();
    painter->setBrush(QColor(0,0,0));
    for(int i=0; i<data->numDimensions; i++)
    {
        a.setX(plotBBox.left() + axesPositions[i]*plotBBox.width());
        b.setX(a.x());

        painter->drawLine(a,b);

        QString text = data->meta[i];
        QPointF center = b - QPointF(fm.width(text)/2,15);
        painter->drawText(center,text);

        text = QString::number(dimMins[i],'g',2);
        center = a - QPointF(fm.width(text)/2,-10);
        painter->drawText(center,text);

        text = QString::number(dimMaxes[i],'g',2);
        center = b - QPointF(fm.width(text)/2,0);
        painter->drawText(center,text);
    }

    // Draw cursor
    painter->setOpacity(1);
    if(cursorPos.x() != -1)
    {
        a.setX(plotBBox.left() + axesPositions[(int)cursorPos.x()]*plotBBox.width() - 10);
        a.setY(cursorPos.y());

        b.setX(plotBBox.left() + axesPositions[(int)cursorPos.x()]*plotBBox.width() + 10);
        b.setY(cursorPos.y());

        painter->drawLine(a,b);
    }

    QVector<QPointF> polyLinePoint;
    polyLinePoint.resize(data->numDimensions);
    qreal boxLeft = plotBBox.left();
    qreal boxBottom = plotBBox.bottom();
    qreal boxWidth = plotBBox.width();
    qreal boxHeight = plotBBox.height();

    int elem = 0;
    QVector<qreal>::Iterator p;

    // Draw unselected elements
    painter->setOpacity(unselOpacity);
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for(p=data->begin; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->selection[elem] != 0)
            continue;

        for(int i=0; i<data->numDimensions; i++)
        {
            a = QPointF(boxLeft+axesPositions[i]*boxWidth,boxBottom);
            a -= QPointF(0,scale(*(p+i),dimMins[i],dimMaxes[i],0,boxHeight));

            polyLinePoint[i] = a;
        }
        painter->drawPolyline(&polyLinePoint[0],polyLinePoint.size());
    }

    painter->setOpacity(selOpacity);
    painter->setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    // Draw selected elements
    elem = 0;
    for(p=data->begin; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->selection[elem] == 0)
            continue;

        for(int i=0; i<data->numDimensions; i++)
        {
            a = QPointF(boxLeft+axesPositions[i]*boxWidth,boxBottom);
            a -= QPointF(0,scale(*(p+i),dimMins[i],dimMaxes[i],0,boxHeight));

            polyLinePoint[i] = a;
        }
        painter->drawPolyline(&polyLinePoint[0],polyLinePoint.size());
    }

    // Draw selection boxes
    painter->setOpacity(0.3);
    for(int i=0; i<data->numDimensions; i++)
    {
        if(selMins[i] != -1)
        {
            a = QPointF(plotBBox.left() + axesPositions[i]*plotBBox.width() - 10,
                        plotBBox.top() + plotBBox.height()*(1.0-selMins[i]));
            b = QPointF(a.x() + 20,
                        plotBBox.top() + plotBBox.height()*(1.0-selMaxes[i]));

            painter->drawRect(QRectF(a,b));
        }
    }

}
