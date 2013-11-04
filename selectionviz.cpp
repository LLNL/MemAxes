#include "selectionviz.h"

#include <iostream>
#include <cmath>
using namespace std;

#include <QVBoxLayout>

SelectionVizWidget::SelectionVizWidget(QWidget *parent) :
    VizWidget(parent)
{
    dim = 0;
}

void SelectionVizWidget::processData()
{
    mins.resize(data->numDimensions);
    maxes.resize(data->numDimensions);
    totals.resize(data->numDimensions);
    means.resize(data->numDimensions);
    stddevs.resize(data->numDimensions);

    selectionMins.resize(data->numDimensions);
    selectionMaxes.resize(data->numDimensions);
    selectionTotals.resize(data->numDimensions);
    selectionMeans.resize(data->numDimensions);
    selStddevs.resize(data->numDimensions);

    qreal firstVal = *(data->begin);
    mins.fill(firstVal);
    maxes.fill(firstVal);

    QVector<qreal>::Iterator p;
    qreal val;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        for(int i=0; i<data->numDimensions; i++)
        {
            val = *(p+i);
            totals[i] += val;
            mins[i] = min(val,mins[i]);
            maxes[i] = max(val,maxes[i]);
        }
    }

    for(int i=0; i<data->numDimensions; i++)
    {
        means[i] = totals[i] / (qreal)data->numElements;
    }

    processed = true;
}

void SelectionVizWidget::selectionChangedSlot()
{
    qreal firstVal = *(data->begin);
    selectionMins.fill(firstVal);
    selectionMaxes.fill(firstVal);

    selectionTotals.fill(0);

    QVector<qreal>::Iterator p;
    int elem;
    qreal val;
    for(elem=0,p=data->begin; p!=data->end; p+=data->numDimensions,elem++)
    {
        if(!data->selected(elem))
            continue;

        for(int i=0; i<data->numDimensions; i++)
        {
            val = *(p+i);
            selectionTotals[i] += val;
            selectionMins[i] = min(val,selectionMins[i]);
            selectionMaxes[i] = max(val,selectionMaxes[i]);
        }
    }

    for(int i=0; i<data->numDimensions; i++)
    {
        selectionMeans[i] = selectionTotals[i] / (qreal)data->numElements;
    }

    repaint();
}

void SelectionVizWidget::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    int m = 20;

    QRect bbox = QRect(m,m,width()-m-m,height()-m-m);
    QPoint cursor = bbox.topLeft() + QPoint(0,15);

    qreal ptotal = selectionTotals[dim]/totals[dim];

    painter->drawText(cursor,"Dimension: "+data->meta[dim]);

    cursor += QPoint(0,240);

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Min: "+QString::number(mins[dim]));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Max: "+QString::number(maxes[dim]));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Mean: "+QString::number(means[dim]));

    cursor = bbox.topLeft() + QPoint(200,245);

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMin: "+QString::number(selectionMins[dim]));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMax: "+QString::number(selectionMaxes[dim]));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMean: "+QString::number(selectionMeans[dim]));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Percent total: "+
                      QString::number(100.0*ptotal));

    // pie chart
    int diameter = min(bbox.width(),bbox.height());
    diameter = min(diameter,200);

    painter->setBrush(QBrush(Qt::blue));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top()+30,diameter,diameter,16*90,16*360);
    painter->setBrush(QBrush(Qt::red));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top()+30,diameter,diameter,16*90,-ptotal*(16*360));
}

void SelectionVizWidget::setDim(int v)
{
    dim = v;
    repaint();
}


