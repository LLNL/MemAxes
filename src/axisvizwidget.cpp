//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#include "axisvizwidget.h"

#include <QPaintEvent>
#include <QMenu>

#include <iostream>
#include <algorithm>

#include "util.h"

AxisVizWidget::AxisVizWidget(QWidget *parent)
    : VizWidget(parent)
{
    // Defaults
    dim = 0;
    numHistBins = 100;

    needsCalcMinMaxes = false;
    needsCalcHistBins = false;
    needsRepaint = false;
}

void AxisVizWidget::frameUpdate()
{
    if(needsCalcMinMaxes)
    {
        calcMinMax();
        needsCalcHistBins = true;
        needsCalcMinMaxes = false;
    }
    if(needsCalcHistBins)
    {
        calcHistBins();
        needsRepaint = true;
        needsCalcHistBins = false;
    }
    if(needsRepaint)
    {
        repaint();
        needsRepaint = false;
    }
}

void AxisVizWidget::selectionChangedSlot()
{

}

void AxisVizWidget::visibilityChangedSlot()
{

}

void AxisVizWidget::setDimension(int d)
{
    dim = d;
    needsCalcMinMaxes = true;
}

void AxisVizWidget::setShowHistograms(bool checked)
{

}

void AxisVizWidget::beginAnimation()
{

}

void AxisVizWidget::endAnimation()
{

}

void AxisVizWidget::requestCluster()
{

}

void AxisVizWidget::resizeEvent(QResizeEvent *e)
{
    needsRepaint = true;
}

void AxisVizWidget::processData()
{
    needsCalcMinMaxes = true;
    processed = true;
}

void AxisVizWidget::drawQtPainter(QPainter *painter)
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
    QPointF b = plotBBox.bottomRight();

    QFontMetrics fm = painter->fontMetrics();
    painter->setBrush(QColor(0,0,0));
    painter->drawLine(a,b);

    QString text = dataSet->meta.at(dim);
    QPointF center = a + QPointF(-fm.width(text)/2,15);
    painter->drawText(center,text);

    text = QString::number(dimMin,'g',2);
    center = a - QPointF(fm.width(text),0);
    painter->drawText(center,text);

    text = QString::number(dimMax,'g',2);
    center = b;
    painter->drawText(center,text);

    // Draw histograms
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(31,120,180));

    qreal left = plotBBox.left();
    qreal histWidth = plotBBox.width() / numHistBins;
    for(int b=0; b<numHistBins; b++)
    {
        qreal height = histVals.at(b)*plotBBox.height();
        qreal top = plotBBox.top()+(plotBBox.height() - height);
        QRectF histRect(left,top,histWidth,height);
        left += histWidth;

        painter->drawRect(histRect);
    }
}

void AxisVizWidget::leaveEvent(QEvent *e)
{

}

void AxisVizWidget::mousePressEvent(QMouseEvent *e)
{

}

void AxisVizWidget::mouseReleaseEvent(QMouseEvent *e)
{

}

bool AxisVizWidget::eventFilter(QObject *obj, QEvent *event)
{
    return true;
}

void AxisVizWidget::calcMinMax()
{
    if(!processed)
        return;

    dimMin = std::numeric_limits<double>::max();
    dimMax = std::numeric_limits<double>::min();

    // Get min and max of visible elements in this dimension
    for(int elem=0; elem<dataSet->numElements; elem++)
    {
        if(!dataSet->visible(elem))
            continue;

        dimMin = std::min(dimMin,dataSet->at(elem,dim));
        dimMax = std::max(dimMax,dataSet->at(elem,dim));
    }
}

void AxisVizWidget::calcHistBins()
{
    if(!processed)
        return;

    // Select histogram bin and increment
    // Gather maximum sized bin along the way
    histMax = 0;
    histVals.resize(100,0);
    for(int elem=0; elem<dataSet->numElements; elem++)
    {
        if(dataSet->selectionDefined() && !dataSet->selected(elem))
            continue;

        int histBin = floor(scale(dataSet->at(elem,dim),dimMin,dimMax,0,numHistBins));

        if(histBin >= numHistBins)
            histBin = numHistBins-1;
        if(histBin < 0)
            histBin = 0;

        histVals[histBin] += 1;
        histMax = std::max(histMax,histVals.at(histBin));
    }

    // Scale hist values to [0,1]
    for(int j=0; j<numHistBins; j++)
        histVals[j] = scale(histVals.at(j),0,histMax,0,1);
}
