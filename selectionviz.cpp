#include "selectionviz.h"

#include <iostream>
#include <cmath>
using namespace std;

#include <QVBoxLayout>

SelectionVizWidget::SelectionVizWidget(QWidget *parent) :
    VizWidget(parent)
{
    dim = 4;
}

void SelectionVizWidget::processData()
{
    data->calcTotalStatistics();
    data->calcSelectionStatistics();
    processed = true;
}

void SelectionVizWidget::selectionChangedSlot()
{
    data->calcSelectionStatistics();
    repaint();
}

void SelectionVizWidget::visibilityChangedSlot()
{
    processData();
    repaint();
}

void SelectionVizWidget::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    int m = 20;

    QRect bbox = QRect(m,m,width()-m-m,height()-m-m);
    QPoint cursor = bbox.topLeft() + QPoint(0,15);

    qreal ptotal = data->selectionSumAt(dim)/data->sumAt(dim);

    painter->drawText(cursor,"Dimension: "+data->meta[dim]);
    cursor += QPoint(0,15);
    painter->drawText(cursor,"Selection: "+QString::number(data->numSelected)+" / "+QString::number(data->numElements));

    cursor += QPoint(0,240);

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Min: "+QString::number(data->minAt(dim)));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Max: "+QString::number(data->maxAt(dim)));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Mean: "+QString::number(data->meanAt(dim)));

    cursor = bbox.topLeft() + QPoint(200,245);

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMin: "+QString::number(data->selectionMinAt(dim)));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMax: "+QString::number(data->selectionMaxAt(dim)));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"selMean: "+QString::number(data->selectionMeanAt(dim)));

    cursor += QPoint(0,15);
    painter->drawText(cursor,"Percent total: "+
                      QString::number(100.0*ptotal));

    // pie chart
    int diameter = min(bbox.width(),bbox.height());
    diameter = min(diameter,200);

    painter->setBrush(QBrush(Qt::gray));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top()+30,diameter,diameter,16*90,16*360);
    painter->setBrush(QBrush(Qt::red));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top()+30,diameter,diameter,16*90,-ptotal*(16*360));
}

void SelectionVizWidget::setDim(int v)
{
    dim = v;
    repaint();
}


