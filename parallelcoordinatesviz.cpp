#include "parallelcoordinatesviz.h"

#include <QPaintEvent>

#include <iostream>
#include <cmath>
using namespace std;

#include "util.h"

ParallelCoordinatesVizWidget::ParallelCoordinatesVizWidget(QWidget *parent)
    : VizWidget(parent)
{
    colorMap.push_back(QColor(166,206,227));
    colorMap.push_back(QColor(31,120,180));
    colorMap.push_back(QColor(178,223,138));
    colorMap.push_back(QColor(51,160,44));
    colorMap.push_back(QColor(251,154,153));
    colorMap.push_back(QColor(227,26,28));
    colorMap.push_back(QColor(253,191,111));
    colorMap.push_back(QColor(255,127,0));
    colorMap.push_back(QColor(202,178,214));
    colorMap.push_back(QColor(106,61,154));
    colorMap.push_back(QColor(255,255,153));
    colorMap.push_back(QColor(177,89,40 ));

    selOpacity = 0.1;
    unselOpacity = 0.01;

    numHistBins = 100;
    showHistograms = false;

    cursorPos.setX(-1);
    selecting = -1;
    movingAxis = -1;

    // Event Filters
    this->installEventFilter(this);
    this->setMouseTracking(true);
}

#define LINES_PER_DATAPT    (numDimensions-1)
#define POINTS_PER_LINE     2
#define FLOATS_PER_POINT    2
#define FLOATS_PER_COLOR    4

void ParallelCoordinatesVizWidget::processData()
{
    processed = false;

    if(dataSet->isEmpty())
        return;

    numDimensions = dataSet->at(0)->numDimensions;

    dimMins.resize(numDimensions);
    dimMaxes.resize(numDimensions);

    dimMins.fill(std::numeric_limits<double>::max());
    dimMaxes.fill(std::numeric_limits<double>::min());

    selMins.resize(numDimensions);
    selMaxes.resize(numDimensions);

    selMins.fill(-1);
    selMaxes.fill(-1);

    axesPositions.resize(numDimensions);
    axesOrder.resize(numDimensions);

    int numTotalElements = 0;
    for(int d=0; d<dataSet->size(); d++)
    {
        numTotalElements += dataSet->at(d)->numElements;
    }

    verts.resize(numTotalElements*LINES_PER_DATAPT*POINTS_PER_LINE*FLOATS_PER_POINT);
    colors.resize(numTotalElements*LINES_PER_DATAPT*POINTS_PER_LINE*FLOATS_PER_COLOR);

    histVals.resize(numDimensions);
    histMaxVals.resize(numDimensions);
    histMaxVals.fill(0);

    // Initial axis positions and order
    for(int i=0; i<numDimensions; i++)
    {
        if(!processed)
            axesOrder[i] = i;

        axesPositions[axesOrder[i]] = i*(1.0/(numDimensions-1));

        histVals[i].resize(numHistBins);
        histVals[i].fill(0);
    }

    processed = true;

    calcMinMaxes();
    calcHistBins();
    recalcLines();
}

void ParallelCoordinatesVizWidget::leaveEvent(QEvent *e)
{
    VizWidget::leaveEvent(e);
    repaint();
}

void ParallelCoordinatesVizWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
    if(!processed)
        return;

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
        for(int i=0; i<numDimensions; i++)
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

    if(!processed)
        return;

    processSelection();

    // No longer selecting
    selMins.fill(-1);
    selMaxes.fill(-1);

    selecting = -1;
    lastSel = -1;
    movingAxis = -1;

    repaint();
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
        }

        // Get cursor location
        cursorPos.setX(-1);
        for(int i=0; i<numDimensions; i++)
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

            for(int i=0; i<numDimensions-1; i++)
            {
                // sort moved axes
                for(int j=i+1; j<numDimensions; j++)
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

            recalcLines();

            if(delta != 0)
                repaint();
        }

        prevMouse = mouseEvent->pos();
    }

    return false;
}

void ParallelCoordinatesVizWidget::processSelection()
{
    QVector<int> selDims;
    QVector<qreal> dataSelMins;
    QVector<qreal> dataSelMaxes;

    if(!processed)
        return;

    for(int i=0; i<selMins.size(); i++)
    {
        if(selMins[i] != -1)
        {
            selDims.push_back(i);
            dataSelMins.push_back(lerp(selMins[i],dimMins[i],dimMaxes[i]));
            dataSelMaxes.push_back(lerp(selMaxes[i],dimMins[i],dimMaxes[i]));
        }
    }

    for(int d=0; d<dataSet->size(); d++)
    {
        if(selDims.isEmpty())
            dataSet->at(d)->deselectAll();
        else
            dataSet->at(d)->selectByMultiDimRange(selDims,dataSelMins,dataSelMaxes);
    }

    recalcLines();

    emit selectionChangedSig();
}

void ParallelCoordinatesVizWidget::calcMinMaxes()
{
    if(!processed)
        return;

    dimMins.fill(std::numeric_limits<double>::max());
    dimMaxes.fill(std::numeric_limits<double>::min());

    int elem;
    QVector<qreal>::Iterator p;
    for(int d=0; d<dataSet->size(); d++)
    {
        for(elem=0, p=dataSet->at(d)->begin; p!=dataSet->at(d)->end; elem++, p+=numDimensions)
        {
            if(!dataSet->at(d)->visible(elem))
                continue;

            for(int i=0; i<numDimensions; i++)
            {
                dimMins[i] = min(dimMins[i],*(p+i));
                dimMaxes[i] = max(dimMaxes[i],*(p+i));
            }
        }
    }
}

void ParallelCoordinatesVizWidget::calcHistBins()
{
    if(!processed)
        return;

    int elem;
    QVector<qreal>::Iterator p;
    for(int d=0; d<dataSet->size(); d++)
    {
        for(elem=0, p=dataSet->at(d)->begin; p!=dataSet->at(d)->end; elem++, p+=numDimensions)
        {
            if(dataSet->at(d)->skip(elem))
                continue;

            for(int i=0; i<numDimensions; i++)
            {
                int histBin = floor(scale(*(p+i),dimMins[i],dimMaxes[i],0,numHistBins));

                if(histBin >= numHistBins)
                    histBin = numHistBins-1;
                if(histBin < 0)
                    histBin = 0;

                histVals[i][histBin] += 1;
                histMaxVals[i] = fmax(histMaxVals[i],histVals[i][histBin]);
            }
        }
    }

    // Scale hist values to [0,1]
    for(int i=0; i<numDimensions; i++)
        for(int j=0; j<numHistBins; j++)
            histVals[i][j] = scale(histVals[i][j],0,histMaxVals[i],0,1);
}

void ParallelCoordinatesVizWidget::recalcLines(int dirtyAxis)
{
    QVector4D col;
    QVector2D a, b;
    int i, axis, nextAxis, elem, idx;
    QVector<double>::Iterator p;

    if(!processed)
        return;

    int allElems = 0;
    for(int d=0; d<dataSet->size(); d++)
    {

        QVector4D redVec = QVector4D(255,0,0,255);
        QColor dataSetColor = valToColor(d,0,dataSet->size(),colorMap);
        qreal Cr,Cg,Cb;
        dataSetColor.getRgbF(&Cr,&Cg,&Cb);
        QVector4D dataColor = QVector4D(Cr,Cg,Cb,1);

        for(p=dataSet->at(d)->begin, elem=0; p!=dataSet->at(d)->end; p+=numDimensions, elem++)
        {
            if(!dataSet->at(d)->visible(elem))
                col = QVector4D(0,0,0,0);
            else if(dataSet->at(d)->selected(elem))
            {
                col = redVec;
                col.setW(selOpacity);
            }
            else
            {
                col = dataColor;
                col.setW(unselOpacity);
            }


            idx = allElems*LINES_PER_DATAPT*POINTS_PER_LINE;
            allElems++;

            for(i=0; i<numDimensions-1; i++)
            {
                if(dirtyAxis != -1  && i != dirtyAxis && i != dirtyAxis-1)
                    continue;

                axis = axesOrder[i];
                nextAxis = axesOrder[i+1];

                float aVal = scale(*(p+axis),dimMins[axis],dimMaxes[axis],0,1);
                a = QVector2D(axesPositions[axis],aVal);

                float bVal = scale(*(p+nextAxis),dimMins[nextAxis],dimMaxes[nextAxis],0,1);
                b = QVector2D(axesPositions[nextAxis],bVal);

                int vertBaseIdx = idx*FLOATS_PER_POINT+i*POINTS_PER_LINE*FLOATS_PER_POINT;
                int colorBaseIdx = idx*FLOATS_PER_COLOR+i*POINTS_PER_LINE*FLOATS_PER_COLOR;

                verts[vertBaseIdx+0] = a.x();
                verts[vertBaseIdx+1] = a.y();

                verts[vertBaseIdx+2] = b.x();
                verts[vertBaseIdx+3] = b.y();

                colors[colorBaseIdx+0] = col.x();
                colors[colorBaseIdx+1] = col.y();
                colors[colorBaseIdx+2] = col.z();
                colors[colorBaseIdx+3] = col.w();

                colors[colorBaseIdx+4] = col.x();
                colors[colorBaseIdx+5] = col.y();
                colors[colorBaseIdx+6] = col.z();
                colors[colorBaseIdx+7] = col.w();
            }
        }
    }
}

void ParallelCoordinatesVizWidget::selectionChangedSlot()
{
    calcHistBins();
    recalcLines();
    repaint();
}

void ParallelCoordinatesVizWidget::visibilityChangedSlot()
{
    processData();
    calcMinMaxes();
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

void ParallelCoordinatesVizWidget::setShowHistograms(bool checked)
{
    showHistograms = checked;
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
    for(int i=0; i<numDimensions; i++)
    {
        a.setX(plotBBox.left() + axesPositions[i]*plotBBox.width());
        b.setX(a.x());

        painter->drawLine(a,b);

        QString text = dataSet->at(0)->meta[i];
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
    int cursorWidth = 28;
    int halfCursorWidth = cursorWidth/2;

    painter->setPen(QPen(Qt::yellow, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    for(int i=0; i<numDimensions; i++)
    {
        if(selMins[i] != -1)
        {
            a = QPointF(plotBBox.left() + axesPositions[i]*plotBBox.width() - halfCursorWidth,
                        plotBBox.top() + plotBBox.height()*(1.0-selMins[i]));
            b = QPointF(a.x() + cursorWidth,
                        plotBBox.top() + plotBBox.height()*(1.0-selMaxes[i]));

            painter->drawRect(QRectF(a,b));
        }
    }

    if(showHistograms)
    {
        // Draw histograms
        a = plotBBox.bottomLeft();
        b = plotBBox.topLeft();

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(31,120,180));
        painter->setOpacity(0.7);

        for(int i=0; i<numDimensions; i++)
        {
            a.setX(plotBBox.left() + axesPositions[i]*plotBBox.width());
            b.setX(a.x());

            for(int j=0; j<numHistBins; j++)
            {
                qreal histTop = a.y()-(j+1)*(plotBBox.height()/numHistBins);
                qreal histLeft = a.x();//-30*histVals[i][j];
                qreal histBottom = a.y()-(j)*(plotBBox.height()/numHistBins);
                qreal histRight = a.x()+60*histVals[i][j];
                painter->drawRect(QRectF(QPointF(histLeft,histTop),QPointF(histRight,histBottom)));
            }

            painter->drawLine(a,b);
        }
    }
}
