#include "correlationmatrixviz.h"

#include <QPainter>
#include <QPaintEvent>

#include <cmath>
#include <iostream>
using namespace std;

#include "util.h"

CorrelationMatrixViz::CorrelationMatrixViz(QWidget *parent)
    : VizWidget(parent)
{
    VizWidget();

    // Set painting variables
    colorBarMin = QColor(0,0,255);
    colorBarMax = QColor(255,0,0);
    minVal = -1;
    maxVal = 1;
    selected = 0;
    highlighted = -1;

    // Event Filters
    this->installEventFilter(this);
    this->setMouseTracking(true);
}

void CorrelationMatrixViz::mouseReleaseEvent(QMouseEvent *event)
{
    static int prevSelected = selected;

    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    QPoint sel = matrixID(mouseEvent->pos());

    if(sel.x() == -1)
        return;

    selected = ROWMAJOR_2D(sel.y(),sel.x(),data->numDimensions);

    if(selected != prevSelected)
    {
        emit selectedDims(sel.x(),sel.y());
        repaint();
    }

    prevSelected = selected;
}

bool CorrelationMatrixViz::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    static int prevHighlighted = highlighted;

    if(!processed)
        return false;

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint sel = matrixID(mouseEvent->pos());

        if(sel.x() == -1)
            highlighted = -1;
        else
            highlighted = ROWMAJOR_2D(sel.y(),sel.x(),data->numDimensions);

        if(highlighted != prevHighlighted)
        {
            repaint();
        }

        prevHighlighted = highlighted;
    }

    return false;
}

void CorrelationMatrixViz::print()
{
    cout << "Printing means : " << endl;
    for(int i=0; i<data->numDimensions; i++)
    {
        cout << expectedValues[i] << "\t\t";
    }
    cout << endl;

    cout << "Printing covariance matrix : " << endl;
    for(int i=0; i<data->numDimensions; i++)
    {
        for(int j=0; j<data->numDimensions; j++)
        {
            cout << covarianceMatrix[ROWMAJOR_2D(i,j,data->numDimensions)] << "\t\t";
        }
        cout << endl;
    }

    cout << "Printing correlation coefficient matrix : " << endl;
    for(int i=0; i<data->numDimensions; i++)
    {
        for(int j=0; j<data->numDimensions; j++)
        {
            cout << correlationMatrix[ROWMAJOR_2D(i,j,data->numDimensions)] << "\t\t";
        }
        cout << endl;
    }
}

void CorrelationMatrixViz::processData()
{
    // ******** Calc correlation coefficients *********
    correlationMatrix.resize(data->numDimensions*data->numDimensions);
    QVector<qreal>::Iterator p;
    qreal x, y;

    // Means and combined means
    QVector<qreal> meanXY;
    expectedValues.resize(data->numDimensions);
    meanXY.resize(data->numDimensions*data->numDimensions);
    expectedValues.fill(0);
    meanXY.fill(0);

    qint64 elem;
    qint64 numselected = 0;
    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->selected(elem))
            numselected++;
    }

    qint64 numelem = (numselected == 0) ? data->numElements : numselected;

    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(numselected > 0 && data->selected(elem) == 0)
            continue;

        for(int i=0; i<data->numDimensions; i++)
        {
            x = *(p+i);
            expectedValues[i] += x;

            for(int j=0; j<data->numDimensions; j++)
            {
                y = *(p+j);
                meanXY[ROWMAJOR_2D(i,j,data->numDimensions)] += x*y;
            }
        }
    }

    // Divide by data->numElements to get mean
    for(int i=0; i<data->numDimensions; i++)
    {
        expectedValues[i] /= (qreal)numelem;
        for(int j=0; j<data->numDimensions; j++)
        {
            meanXY[ROWMAJOR_2D(i,j,data->numDimensions)] /= (qreal)numelem;
        }
    }

    // Covariance = E(XY) - E(X)*E(Y)
    // TODO: Possibly switch to E((x-E(x))*(y-E(y)) to avoid floating-point error
    covarianceMatrix.resize(data->numDimensions*data->numDimensions);

    for(int i=0; i<data->numDimensions; i++)
    {
        for(int j=0; j<data->numDimensions; j++)
        {
            covarianceMatrix[ROWMAJOR_2D(i,j,data->numDimensions)] =
                meanXY[ROWMAJOR_2D(i,j,data->numDimensions)] - expectedValues[i]*expectedValues[j];
        }
    }

    // Standard deviation of each dim
    standardDeviations.resize(data->numDimensions);
    standardDeviations.fill(0);

    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(numselected > 0 && data->selected(elem) == 0)
            continue;

        for(int i=0; i<data->numDimensions; i++)
        {
            x = *(p+i);
            standardDeviations[i] += (x-expectedValues[i])*(x-expectedValues[i]);
        }
    }

    for(int i=0; i<data->numDimensions; i++)
    {
        standardDeviations[i] = sqrt(standardDeviations[i]/(qreal)numelem);
    }

    // Correlation Coeff = cov(xy) / stdev(x)*stdev(y)
    for(int i=0; i<data->numDimensions; i++)
    {
        for(int j=0; j<data->numDimensions; j++)
        {
            correlationMatrix[ROWMAJOR_2D(i,j,data->numDimensions)] =
                    covarianceMatrix[ROWMAJOR_2D(i,j,data->numDimensions)] /
                    (standardDeviations[i]*standardDeviations[j]);
        }
    }

    processed = true;
}

QPoint CorrelationMatrixViz::matrixID(QPoint pixel)
{
    if(!matrixBBox.contains(pixel))
        return QPoint(-1,-1);

    qreal sx = data->numDimensions*normalize(pixel.x(),matrixBBox.left(),matrixBBox.right());
    qreal sy = data->numDimensions*normalize(pixel.y(),matrixBBox.top(),matrixBBox.bottom());

    return QPoint(floor(sx),floor(sy));
}

void CorrelationMatrixViz::leaveEvent(QEvent *e)
{
    VizWidget::leaveEvent(e);
    repaint();
}

void CorrelationMatrixViz::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    qreal m = 20;
    qreal lm = 50;
    qreal tw = 70;

    matrixBBox = QRect(lm,m,
                       (rect().right()-m-tw) - (rect().left()+lm),
                       (rect().bottom()-m) - (rect().top()+m));

    qreal deltax = matrixBBox.width() / data->numDimensions;
    qreal deltay = matrixBBox.height() / data->numDimensions;

    QPointF o = matrixBBox.topLeft();

    // Color bar
    QRectF colorBarRect(10,20,20,70);
    QLinearGradient colorBar(0,colorBarRect.top(),0,colorBarRect.height());

    colorBar.setColorAt(0.0,colorBarMax);
    colorBar.setColorAt(1.0,colorBarMin);
    //colorBar.setInterpolationMode(QGradient::ComponentInterpolation);

    painter->setBrush(colorBar);
    painter->drawRect(colorBarRect);

    painter->setBrush(QBrush(QColor(0,0,0)));
    painter->drawText(colorBarRect.topLeft()+QPointF(-5,-2),QString::number(maxVal,'g',2));
    painter->drawText(colorBarRect.bottomLeft()+QPointF(-5,12),QString::number(minVal,'g',2));

    // Vertical lines
    QPointF a = o;
    QPointF b = matrixBBox.bottomLeft();

    painter->setBrush(QBrush(QColor(0,0,0)));

    for(int i=0; i<=data->numDimensions; i++)
    {
        painter->drawLine(a,b);
        a += QPointF(deltax,0);
        b += QPointF(deltax,0);
    }

    // Horizontal Lines
    a = o;
    b = matrixBBox.topRight() + QPointF(tw,0);

    for(int i=0; i<=data->numDimensions; i++)
    {
        painter->drawLine(a,b);
        a += QPointF(0,deltay);
        b += QPointF(0,deltay);
    }

    // Draw matrix values
    painter->setPen(QColor(0,0,0));
    a = matrixBBox.topLeft();
    b = a + QPointF(deltax,deltay);
    for(int i=0; i<data->numDimensions; i++)
    {
        a = o + QPointF(0,i*deltay);
        b = a + QPointF(deltax,deltay);
        for(int j=0; j<data->numDimensions; j++)
        {
            painter->setBrush(
                        valToColor(correlationMatrix[ROWMAJOR_2D(i,j,data->numDimensions)],
                                   minVal, maxVal, colorBarMin, colorBarMax));

            if(ROWMAJOR_2D(i,j,data->numDimensions) == selected)
            {
                painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->drawRect(QRectF(a+QPointF(2,2),b-QPointF(2,2)));
            }
            else if(ROWMAJOR_2D(i,j,data->numDimensions) == highlighted)
            {
                painter->setPen(QPen(Qt::yellow, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->drawRect(QRectF(a+QPointF(2,2),b-QPointF(2,2)));
            }
            else
            {
                painter->setPen(QColor(0,0,0));
                painter->drawRect(QRectF(a,b));
            }

            a += QPointF(deltax,0);
            b += QPointF(deltax,0);
        }
    }

    // Draw labels
    painter->setPen(QColor(0,0,0));
    for(int i=0; i<data->numDimensions; i++)
    {
        painter->drawText(o+QPointF(i*deltax,-2),data->meta[i]);
    }

    QPointF vp = matrixBBox.topRight();
    for(int i=0; i<data->numDimensions; i++)
    {
        painter->drawText(vp+QPointF(0,10),data->meta[i]);
        vp += QPointF(0,deltay);
    }
}

void CorrelationMatrixViz::setMin(int v)
{
    // Assumes slider is 0-99
    qreal vn = normalize(v,0,99);
    minVal = lerp(vn,-1.0,1.0);
    repaint();
}

void CorrelationMatrixViz::setMax(int v)
{
    // Assumes slider is 0-99
    qreal vn = normalize(v,0,99);
    maxVal = lerp(vn,-1.0,1.0);
    repaint();
}

void CorrelationMatrixViz::selectionChangedSlot()
{
    processData();
    repaint();
}
