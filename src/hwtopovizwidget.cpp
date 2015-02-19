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

#include "hwtopovizwidget.h"

#include <iostream>
#include <cmath>

using namespace std;

HWTopoVizWidget::HWTopoVizWidget(QWidget *parent) :
    VizWidget(parent)
{
    dataMode = COLORBY_CYCLES;
    vizMode = SUNBURST;

    colorMap = gradientColorMap(QColor(255,237,160),
                                QColor(240,59 ,32 ),
                                256);

    this->installEventFilter(this);
    setMouseTracking(true);
}

void HWTopoVizWidget::frameUpdate()
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    if(needsCalcMinMaxes)
    {
        calcMinMaxes();
        needsCalcMinMaxes = false;
    }
    if(needsConstructNodeBoxes)
    {
        constructNodeBoxes(drawBox,
                           dataSet->getTopo(),
                           depthValRanges,
                           depthTransRanges,
                           dataMode,
                           nodeBoxes,linkBoxes);
        needsConstructNodeBoxes = false;
    }
    if(needsRepaint)
    {
        repaint();
        needsRepaint = false;
    }
}

void HWTopoVizWidget::processData()
{
    processed = false;

    if(dataSet->getTopo() == NULL)
        return;

    depthRange = IntRange(0,dataSet->getTopo()->hardwareResourceMatrix.size());
    for(int i=depthRange.first; i<(int)depthRange.second; i++)
    {
        IntRange wr(0,dataSet->getTopo()->hardwareResourceMatrix[i].size());
        widthRange.push_back(wr);
    }

    processed = true;

    needsCalcMinMaxes = true;
}

void HWTopoVizWidget::selectionChangedSlot()
{
    if(!processed)
        return;

    needsCalcMinMaxes = true;
}

void HWTopoVizWidget::visibilityChangedSlot()
{
    if(!processed)
        return;

    needsCalcMinMaxes = true;
}

void HWTopoVizWidget::drawTopo(QPainter *painter, QRectF rect, ColorMap &cm, QVector<NodeBox> &nb, QVector<LinkBox> &lb)
{
    // Draw node outlines
    painter->setPen(QPen(Qt::black));
    for(int b=0; b<nb.size(); b++)
    {
        hwNode *node = nb.at(b).node;
        QRectF box = nb.at(b).box;
        QString text = QString::number(node->id);

        // Color by value
        QColor col = valToColor(nb.at(b).val,cm);
        painter->setBrush(col);

        // Draw rect (radial or regular)
        if(vizMode == SUNBURST)
        {
            QVector<QPointF> segmentPoly = rectToRadialSegment(box,rect);
            painter->drawPolygon(segmentPoly.constData(),segmentPoly.size());
        }
        else if(vizMode == ICICLE)
        {
            painter->drawRect(box);
            QPointF center = box.center() - QPointF(4,-4);
            painter->drawText(center,text);
        }
    }

    // Draw links
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    for(int b=0; b<lb.size(); b++)
    {
        QRectF box = lb[b].box;

        if(vizMode == SUNBURST)
        {
            QVector<QPointF> segmentPoly = rectToRadialSegment(box,rect);
            painter->drawPolygon(segmentPoly.constData(),segmentPoly.size());
        }
        else if(vizMode == ICICLE)
        {
            painter->drawRect(box);
        }
    }
}

void HWTopoVizWidget::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    drawTopo(painter,drawBox,colorMap,nodeBoxes,linkBoxes);

}

void HWTopoVizWidget::mousePressEvent(QMouseEvent *e)
{
    if(!processed)
        return;

    hwNode *node = nodeAtPosition(e->pos());

    if(node)
    {
        selectSamplesWithinNode(node);
    }

}

void HWTopoVizWidget::mouseMoveEvent(QMouseEvent* e)
{
    if(!processed)
        return;

    hwNode *node = nodeAtPosition(e->pos());

    if(node)
    {
        QString label;

        if(node->depth == dataSet->getTopo()->totalDepth)
            label = "CPU " + QString::number(node->id) + "\n";
        else if(node->depth > 1)
            label = "L" + QString::number(node->id) + " Cache\n";
        else if(node->depth == 1)
            label = "NUMA Node " + QString::number(node->id) + "\n";
        else
            label = "RAM\n";

        label += "\n";
        label += "Size: " + QString::number(node->size) + " bytes\n";

        label += "\n";

        int numCycles = 0;
        int numSamples = 0;
        numSamples += node->sampleSets[dataSet].selSamples.size();
        numCycles += node->sampleSets[dataSet].selCycles;

        label += "Samples: " + QString::number(numSamples) + "\n";
        label += "Cycles: " + QString::number(numCycles) + "\n";

        label += "\n";
        label += "Cycles/Access: " + QString::number((float)numCycles / (float)numSamples) + "\n";

        QToolTip::showText(e->globalPos(),label,this, rect() );
    }
    else
    {
        QToolTip::hideText();
    }
}

void HWTopoVizWidget::resizeEvent(QResizeEvent *e)
{
    VizWidget::resizeEvent(e);

    if(!processed)
        return;

    needsConstructNodeBoxes = true;
    frameUpdate();
}

void HWTopoVizWidget::calcMinMaxes()
{
    RealRange limits;
    limits.first = 99999999;
    limits.second = 0;

    depthValRanges.resize(depthRange.second - depthRange.first);
    depthValRanges.fill(limits);

    depthTransRanges.resize(depthRange.second - depthRange.first);
    depthTransRanges.fill(limits);

    for(int r=0, i=depthRange.first; i<depthRange.second; r++, i++)
    {
        // Get min/max for this row
        for(int j=widthRange[r].first; j<widthRange[r].second; j++)
        {
            hwNode *node = dataSet->getTopo()->hardwareResourceMatrix[i][j];

            if(!node->sampleSets.contains(dataSet))
                continue;

            ElemSet &samples = node->sampleSets[dataSet].selSamples;
            int *numCycles = &node->sampleSets[dataSet].selCycles;

            qreal val = (dataMode == COLORBY_CYCLES) ? *numCycles : samples.size();
            //val = (qreal)(*numCycles) / (qreal)samples->size();

            depthValRanges[i].first=0;//min(depthValRanges[i].first,val);
            depthValRanges[i].second=max(depthValRanges[i].second,val);

            qreal trans = node->transactions;
            depthTransRanges[i].first=0;//min(depthTransRanges[i].first,trans);
            depthTransRanges[i].second=max(depthTransRanges[i].second,trans);
        }
    }

    needsConstructNodeBoxes = true;
    needsRepaint = true;
}

void HWTopoVizWidget::constructNodeBoxes(QRectF rect,
                                    hwTopo *topo,
                                    QVector<RealRange> &valRanges,
                                    QVector<RealRange> &transRanges,
                                    DataMode m,
                                    QVector<NodeBox> &nbout,
                                    QVector<LinkBox> &lbout)
{
    nbout.clear();
    lbout.clear();

    if(topo == NULL)
        return;

    float nodeMarginX = 2.0f;
    float nodeMarginY = 10.0f;

    float deltaX = 0;
    float deltaY = rect.height() / topo->hardwareResourceMatrix.size();

    // Adjust boxes to fill the rect space
    for(int i=0; i<topo->hardwareResourceMatrix.size(); i++)
    {
        deltaX = rect.width() / (float)topo->hardwareResourceMatrix[i].size();
        for(int j=0; j<topo->hardwareResourceMatrix[i].size(); j++)
        {
            // Create Node Box
            NodeBox nb;
            nb.node = topo->hardwareResourceMatrix[i][j];
            nb.box.setRect(rect.left()+j*deltaX,
                           rect.top()+i*deltaY,
                           deltaX,
                           deltaY);

            // Get value by cycles or samples
            int numCycles = 0;
            int numSamples = 0;
            numSamples += nb.node->sampleSets[dataSet].selSamples.size();
            numCycles += nb.node->sampleSets[dataSet].selCycles;

            qreal unscaledval = (m == COLORBY_CYCLES) ? numCycles : numSamples;
            nb.val = scale(unscaledval,
                           valRanges.at(i).first,
                           valRanges.at(i).second,
                           0, 1);

            if(i==0)
                nb.box.adjust(0,0,0,-nodeMarginY);
            else
                nb.box.adjust(nodeMarginX,nodeMarginY,-nodeMarginX,-nodeMarginY);

            nbout.push_back(nb);

            // Create Link Box
            if(i-1 >= 0)
            {
                LinkBox lb;
                lb.parent = nb.node->parent;
                lb.child = nb.node;

                // scale width by transactions
                lb.box.setRect(rect.left()+j*deltaX,
                               rect.top()+i*deltaY,
                               deltaX,
                               nodeMarginY);

                lb.box.adjust(nodeMarginX,-nodeMarginY,-nodeMarginX,0);
                
                float linkWidth = scale(nb.node->transactions,
                                        transRanges.at(i).first,
                                        transRanges.at(i).second,
                                        1.0f,
                                        lb.box.width());
                float deltaWidth = (lb.box.width()-linkWidth)/2.0f;

                lb.box.adjust(deltaWidth,0,-deltaWidth,0);

                lbout.push_back(lb);
            }
        }
    }

    needsRepaint = true;
}

hwNode *HWTopoVizWidget::nodeAtPosition(QPoint p)
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    for(int b=0; b<nodeBoxes.size(); b++)
    {
        hwNode *node = nodeBoxes[b].node;
        QRectF box = nodeBoxes[b].box;

        bool containsP = false;
        if(vizMode == SUNBURST)
        {
            QPointF radp = reverseRadialTransform(p,drawBox);
            containsP = box.contains(radp);
        }
        else if(vizMode == ICICLE)
        {
            containsP = box.contains(p);
        }

        if(containsP)
            return node;
    }

    return NULL;
}

void HWTopoVizWidget::selectSamplesWithinNode(hwNode *node)
{
    dataSet->selectByResource(node);
    emit selectionChangedSig();
}
