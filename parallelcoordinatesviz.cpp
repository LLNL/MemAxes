#include "parallelcoordinatesviz.h"

#include <QPaintEvent>

#include <iostream>
#include <cmath>
using namespace std;

#include "util.h"

ParallelCoordinatesVizWidget::ParallelCoordinatesVizWidget(QWidget *parent)
    : VizWidget(parent)
{
    cursorPos.setX(-1);
    selecting = -1;
    movingAxis = -1;

    selOpacity = 0.1;
    unselOpacity = 0.01;

    this->setMaximumHeight(200);

    // Event Filters
    this->installEventFilter(this);
    this->setMouseTracking(true);

    // QGL
    QColor qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
    qglClearColor(qtPurple.light());

}

#define LINES_PER_DATAPT    (data->numDimensions-1)
#define POINTS_PER_LINE     2
#define FLOATS_PER_POINT    2
#define FLOATS_PER_COLOR    4

void ParallelCoordinatesVizWidget::processData()
{
    dimMins.resize(data->numDimensions);
    dimMaxes.resize(data->numDimensions);

    selMins.resize(data->numDimensions);
    selMaxes.resize(data->numDimensions);

    selMins.fill(-1);
    selMaxes.fill(-1);

    axesPositions.resize(data->numDimensions);
    axesOrder.resize(data->numDimensions);

    verts.resize(data->numElements*LINES_PER_DATAPT*POINTS_PER_LINE*FLOATS_PER_POINT);
    colors.resize(data->numElements*LINES_PER_DATAPT*POINTS_PER_LINE*FLOATS_PER_COLOR);

    for(int i=0; i<data->numDimensions; i++)
    {
        dimMins[i] = *(data->begin+i);
        dimMaxes[i] = *(data->begin+i);

        axesOrder[i] = i;
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

    recalcLines();

    processed = true;
}

void ParallelCoordinatesVizWidget::leaveEvent(QEvent *e)
{
    VizWidget::leaveEvent(e);
    repaint();
}

void ParallelCoordinatesVizWidget::mousePressEvent(QMouseEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

    if(cursorPos.x() != -1)
    {
        selecting = cursorPos.x();
        firstSel = mouseEvent->pos().y();
    }

    if(mouseEvent->pos().y() > 0
            && mouseEvent->pos().y() < plotBBox.top())
    {
        int xval = mouseEvent->pos().x();

        qreal dist;
        int closestAxis = plotBBox.width();
        for(int i=0; i<data->numDimensions; i++)
        {
            dist = abs(40+axesPositions[i]*plotBBox.width()-xval);
            if(dist < closestAxis)
            {
                closestAxis = dist;
                movingAxis = i;
            }
        }
    }
}

void ParallelCoordinatesVizWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

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
    movingAxis = -1;
}

bool ParallelCoordinatesVizWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    if(!processed)
        return false;

    static qreal prevCursor = cursorPos.x();
    static QPoint prevMouse;

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

        // Move axes
        if(movingAxis != -1)
        {
            qreal delta = mouseEvent->pos().x() - prevMouse.x();
            axesPositions[movingAxis] += delta/plotBBox.width();

            for(int i=0; i<data->numDimensions-1; i++)
            {
                // sort moved axes
                for(int j=i+1; j<data->numDimensions; j++)
                {
                    if(     axesPositions[axesOrder[j]] <
                            axesPositions[axesOrder[i]])
                    {
                        int tmp = axesOrder[j];
                        axesOrder[j] = axesOrder[i];
                        axesOrder[i] = tmp;
                    }
                }
            }

            recalcLines(movingAxis);

            if(delta != 0)
                repaint();
        }

        prevMouse = mouseEvent->pos();
    }

    return false;
}

void ParallelCoordinatesVizWidget::processSelection()
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
        data->deselectAll();
    }
    else
    {
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
            if(select == selAxes)
                data->selectData(elem,1);
            else
                data->deselectData(elem);
            elem++;
        }
    }

    recalcLines();

    emit selectionChangedSig();
}

void ParallelCoordinatesVizWidget::recalcLines(int dirtyAxis)
{
    QVector4D col;
    QVector2D a, b;
    int i, axis, nextAxis, elem, idx;
    QVector<double>::Iterator p;

    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->selected(elem) != 0)
            col = QVector4D(1,0,0,selOpacity);
        else
            col = QVector4D(0,0,0,unselOpacity);

        idx = elem*LINES_PER_DATAPT*POINTS_PER_LINE;

        for(i=0; i<data->numDimensions-1; i++)
        {
            if(dirtyAxis != -1  && i != dirtyAxis && i != dirtyAxis-1)
                continue;

            axis = axesOrder[i];
            nextAxis = axesOrder[i+1];

            a = QVector2D(axesPositions[axis],
                          scale(*(p+axis),dimMins[axis],dimMaxes[axis],0,1));

            b = QVector2D(axesPositions[nextAxis],
                          scale(*(p+nextAxis),dimMins[nextAxis],dimMaxes[nextAxis],0,1));

            verts[idx*FLOATS_PER_POINT+i*POINTS_PER_LINE*FLOATS_PER_POINT+0] = a.x();
            verts[idx*FLOATS_PER_POINT+i*POINTS_PER_LINE*FLOATS_PER_POINT+1] = a.y();

            verts[idx*FLOATS_PER_POINT+i*POINTS_PER_LINE*FLOATS_PER_POINT+2] = b.x();
            verts[idx*FLOATS_PER_POINT+i*POINTS_PER_LINE*FLOATS_PER_POINT+3] = b.y();

            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+0] = col.x();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+1] = col.y();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+2] = col.z();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+3] = col.w();

            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+4] = col.x();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+5] = col.y();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+6] = col.z();
            colors[idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR+7] = col.w();
        }
    }
}

void ParallelCoordinatesVizWidget::selectionChangedSlot()
{
    recalcLines();
    repaint();
}

void ParallelCoordinatesVizWidget::setSelOpacity(int val)
{
    selOpacity = (qreal)val/1000.0;
    recalcLines();
    repaint();
}

void ParallelCoordinatesVizWidget::setUnselOpacity(int val)
{
    unselOpacity = (qreal)val/1000.0;
    recalcLines();
    repaint();
}

void ParallelCoordinatesVizWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if(!processed)
        return;

    makeCurrent();

    int mx=40;
    int my=30;

    // Qt 5.X BUG
    //int width2x = width()*2;
    //int height2x = height()*2;
    //int mx2x = mx*2;
    //int my2x = my*2;
    //glViewport(mx2x,
    //           my2x,
    //           width2x-2*mx2x,
    //           height2x-2*my2x);

    glViewport(mx,
               my,
               width()-2*mx,
               height()-2*my);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0, 1);

    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(FLOATS_PER_POINT,GL_FLOAT,0,verts.constData());
    glColorPointer(FLOATS_PER_COLOR,GL_FLOAT,0,colors.constData());

    glDrawArrays(GL_LINES,0,verts.size()/POINTS_PER_LINE);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void ParallelCoordinatesVizWidget::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    int mx=40;
    int my=30;

    plotBBox = QRectF(mx,my,
                      width()-mx-mx,
                      height()-my-my);

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
