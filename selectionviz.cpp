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
    int rectWidth = 15;
    float selSumData,totalSumData;

    if(mode == WEIGHTBY_CYCLES)
    {
        int dim = data->meta.indexOf("latency");
        selSumData = data->selectionSumAt(dim);
        totalSumData = data->sumAt(dim);
    }
    else if(mode == WEIGHTBY_SAMPLES)
    {
        selSumData = data->numSelected;
        totalSumData = data->numElements;
    }
    else
    {
        cerr << "COCKATOOS" << endl;
        return;
    }

    QRect bbox = QRect(m,m,width()-m-m,height()-m-m);
    qreal selectionPercent = selSumData/totalSumData;
    qreal unselectionPercent = 1.0 - selectionPercent;

    QString blueLabel = QString::number(unselectionPercent*100) + "% (" +
                        QString::number(data->numElements-data->numSelected) + " accesses)";

    QString redLabel = QString::number(selectionPercent*100) + "% (" +
                        QString::number(data->numSelected) + " accesses)";

    QPoint cursor = bbox.bottomLeft();
    cursor -= QPoint(0,20);

    painter->setBrush(QBrush(Qt::gray));
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawRect(cursor.x(), cursor.y(), rectWidth, rectWidth);
    painter->drawText(QPoint(cursor.x()+rectWidth+5,cursor.y()+rectWidth),blueLabel);

    cursor = QPoint(cursor.x(),cursor.y()+20);

    painter->setBrush(QBrush(QColor(178,24,43)));
    painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter->drawRect(cursor.x(), cursor.y(), rectWidth, rectWidth);
    painter->drawText(QPoint(cursor.x()+rectWidth+5,cursor.y()+rectWidth),redLabel);

    // pie chart
    int diameter = min(bbox.width(),bbox.height())-25;

    painter->setBrush(QBrush(Qt::gray));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top(),diameter,diameter,16*90,16*360);
    painter->setBrush(QBrush(QColor(178,24,43)));
    painter->drawPie(bbox.center().x()-diameter/2,bbox.top(),diameter,diameter,16*90,-selectionPercent*(16*360));
}
