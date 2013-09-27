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
    movingAxis = -1;

    selOpacity = 0.1;
    unselOpacity = 0.01;

    // Event Filters
    this->installEventFilter(this);
    this->setMouseTracking(true);

    // QGL
    QColor qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
    qglClearColor(qtPurple.light());

    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
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

    linePositions.resize(data->numElements*(data->numDimensions-1)*2);
    lineColors.resize(data->numElements*(data->numDimensions-1)*2);

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
    movingAxis = -1;
}

bool ParallelCoordinatesViz::eventFilter(QObject *obj, QEvent *event)
{
    if(!vizProcessed)
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

            if(delta != 0)
                repaint();
        }

        prevMouse = mouseEvent->pos();
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
            data->selection[elem] = (select == selAxes) ? 1 : 0;
            elem++;
        }
    }

    emit selectionChangedSig();
}

void ParallelCoordinatesViz::recalcLines(QRect rect, int dirtyAxis)
{
    QVector4D col;
    QVector2D a, b;
    int i, axis, nextAxis, elem, idx;
    QVector<double>::Iterator p;

    int mx=40;
    int my=30;

    QRect box = QRect(mx+mx,my+my,
                      rect.width()-mx-mx-mx-mx,
                      rect.height()-my-my-my-my);

    qreal boxLeft = box.left();
    qreal boxBottom = box.bottom();
    qreal boxWidth = box.width();
    qreal boxHeight = box.height();

    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->selection[elem] != 0)
            col = QVector4D(1,0,0,selOpacity);
        else
            col = QVector4D(0,0,0,unselOpacity);

        idx = elem*(data->numDimensions-1)*2;

        for(i=0; i<data->numDimensions-1; i++)
        {
            if(dirtyAxis != -1  && i != dirtyAxis)
                continue;

            axis = axesOrder[i];
            nextAxis = axesOrder[i+1];

            a = QVector2D(boxLeft+axesPositions[axis]*boxWidth,boxBottom);
            a -= QVector2D(0,scale(*(p+axis),dimMins[axis],dimMaxes[axis],0,boxHeight));

            b = QVector2D(boxLeft+axesPositions[nextAxis]*boxWidth,boxBottom);
            b -= QVector2D(0,scale(*(p+nextAxis),dimMins[nextAxis],dimMaxes[nextAxis],0,boxHeight));

            linePositions[idx+i*2] = a;
            linePositions[idx+i*2+1] = b;

            lineColors[idx+i*2] = col;
            lineColors[idx+i*2+1] = col;
        }
    }
}

void ParallelCoordinatesViz::paintGL(QRect rect)
{
    makeCurrent();

    qglClearColor(backgroundColor.color());
    glClear(GL_COLOR_BUFFER_BIT);

    rect.setWidth(rect.width()*2);
    rect.setHeight(rect.height()*2);

    // Set up matrices for 2d drawing
    glViewport(0,0,rect.width(),rect.height());

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, rect.width(), rect.height(), 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw elements
    recalcLines(rect);

    if(0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(2,GL_DOUBLE,0,linePositions.constData());
        glColorPointer(4,GL_DOUBLE,0,lineColors.constData());

        glDrawArrays(GL_LINES,0,linePositions.size()/2);
    }
    else
    {
        glBegin(GL_LINES);
        for(int i=0; i<linePositions.size(); i++)
        {
            glColor4f(lineColors[i].x(),lineColors[i].y(),lineColors[i].z(),lineColors[i].w());
            glVertex2f(linePositions[i].x(),linePositions[i].y());
        }
        glEnd();
    }

    // Reset matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void ParallelCoordinatesViz::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{
    //painter->fillRect(event->rect(), backgroundColor);

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
