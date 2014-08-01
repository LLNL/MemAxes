#include "selectionviz.h"

#include <iostream>
#include <cmath>
using namespace std;

#include <QVBoxLayout>

SelectionVizWidget::SelectionVizWidget(QWidget *parent) :
    VizWidget(parent)
{
    mode = WEIGHTBY_CYCLES;
}

void SelectionVizWidget::processData()
{
    processed = false;

    if(dataSet->isEmpty())
        return;

    processed = true;
}

void SelectionVizWidget::selectionChangedSlot()
{
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
    int rectWidth = 15;
    float selSumCycles,selSumSamples,totalSumCycles,totalSumSamples;
    int dim = dataSet->at(0)->meta.indexOf("latency");

    selSumCycles = dataSet->at(0)->selectionSumAt(dim);
    totalSumCycles = dataSet->at(0)->sumAt(dim);

    selSumSamples = dataSet->at(0)->numSelected;
    totalSumSamples = dataSet->at(0)->numElements;

    qreal selectionPercent;

    if(mode == WEIGHTBY_CYCLES)
        selectionPercent = selSumCycles/totalSumCycles;
    else if(mode == WEIGHTBY_SAMPLES)
        selectionPercent = selSumSamples/totalSumSamples;
    else
    {
        cerr << "COCKATOOS" << endl;
        return;
    }

    qreal unselectionPercent = 1.0 - selectionPercent;

    QRect bbox = QRect(m,m,width()-m-m,height()-m-m);

    QString blueLabel = QString::number(unselectionPercent*100) + "% (" +
                        QString::number(dataSet->at(0)->numElements-dataSet->at(0)->numSelected) + " accesses)";

    QString redLabel = QString::number(selectionPercent*100) + "% (" +
                       QString::number(dataSet->at(0)->numSelected) + " accesses)";

    QString avgLabel = QString::number(selSumCycles/selSumSamples) +
                       " Cycles/Access";

    QPoint cursor = bbox.bottomLeft();
    cursor -= QPoint(0,40);

    painter->setBrush(QBrush(Qt::gray));
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawRect(cursor.x(), cursor.y(), rectWidth, rectWidth);
    painter->drawText(QPoint(cursor.x()+rectWidth+5,cursor.y()+rectWidth),blueLabel);

    cursor = QPoint(cursor.x(),cursor.y()+20);

    painter->setBrush(QBrush(QColor(178,24,43)));
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawRect(cursor.x(), cursor.y(), rectWidth, rectWidth);
    painter->drawText(QPoint(cursor.x()+rectWidth+5,cursor.y()+rectWidth),redLabel);

    cursor = QPoint(cursor.x(),cursor.y()+20+rectWidth);

    painter->setBrush(QBrush(QColor(178,24,43)));
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawText(cursor,avgLabel);


    // pie chart
    int diameter = min(bbox.width(),bbox.height())-25;

    painter->setBrush(QBrush(Qt::gray));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top(),diameter,diameter,16*90,16*360);
    painter->setBrush(QBrush(QColor(178,24,43)));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top(),diameter,diameter,16*90,-selectionPercent*(16*360));
}
