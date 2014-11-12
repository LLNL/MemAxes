//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/rememory/MemAxes
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
#include "memtopoviz.h"

#include <iostream>
#include <cmath>

using namespace std;

MemTopoViz::MemTopoViz(QWidget *parent) :
    VizWidget(parent)
{
    dataMode = COLORBY_CYCLES;
    vizMode = SUNBURST;

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

    this->installEventFilter(this);
    setMouseTracking(true);
}

void MemTopoViz::processData()
{
    processed = false;

    if(dataSet->isEmpty() || !dataSet->hwTopo())
        return;

    depthRange = IntRange(0,dataSet->hwTopo()->hardwareResourceMatrix.size());
    for(int i=depthRange.first; i<(int)depthRange.second; i++)
    {
        IntRange wr(0,dataSet->hwTopo()->hardwareResourceMatrix[i].size());
        widthRange.push_back(wr);
    }

    processed = true;

    calcMinMaxes();
    resizeNodeBoxes();
    repaint();
}

void MemTopoViz::selectionChangedSlot()
{
    if(!processed)
        return;

    calcMinMaxes();
    resizeNodeBoxes();
    repaint();
}

void MemTopoViz::visibilityChangedSlot()
{
    if(!processed)
        return;

    calcMinMaxes();
    resizeNodeBoxes();
    repaint();
}

void MemTopoViz::drawQtPainter(QPainter *painter)
{
    if(!processed)
        return;

    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    QPen pen(Qt::black, 1, Qt::SolidLine);
    //QPen::QPen ( const QBrush & brush, qreal width,
    //             Qt::PenStyle style = Qt::SolidLine,
    //             Qt::PenCapStyle cap = Qt::SquareCap,
    //             Qt::PenJoinStyle join = Qt::BevelJoin )

    // Draw data boxes first
    painter->setPen(Qt::NoPen);
    for(int b=0; b<nodeDataBoxes.size(); b++)
    {
        QColor col = nodeDataBoxes[b].first;
        QRectF box = nodeDataBoxes[b].second;

        painter->setBrush(col);

        if(vizMode == SUNBURST)
        {
            QVector<QPointF> segmentPoly = rectToRadialSegment(box,drawBox);
            painter->drawPolygon(segmentPoly.constData(),segmentPoly.size());
        }
        else if(vizMode == ICICLE)
        {
            painter->drawRect(box);
        }
    }

    // Draw node outlines
    painter->setBrush(Qt::NoBrush);
    painter->setPen(pen);
    for(int b=0; b<nodeBoxes.size(); b++)
    {
        hardwareResourceNode *node = nodeBoxes[b].first;
        QRectF box = nodeBoxes[b].second;
        QString text = QString::number(node->id);

        if(vizMode == SUNBURST)
        {
            QVector<QPointF> segmentPoly = rectToRadialSegment(box,drawBox);
            painter->drawPolygon(segmentPoly.constData(),segmentPoly.size());
        }
        else if(vizMode == ICICLE)
        {
            painter->drawRect(box);
            QPointF center = box.center() - QPointF(4,-4);
            painter->drawText(center,text);
        }
    }
}

void MemTopoViz::mousePressEvent(QMouseEvent *e)
{
    if(!processed)
        return;

    hardwareResourceNode *node = nodeAtPosition(e->pos());

    if(node)
    {
        selectSamplesWithinNode(node);
    }

}

void MemTopoViz::zoomVertical(hardwareResourceNode *node, int dir)
{
    hardwareTopology *topo = dataSet->hwTopo();
    int d = node->depth;
    int range = depthRange.second - depthRange.first;

    if(range == 1 && dir > 0)
        return;
    if(range < 1)
        return;

    dir = (dir > 0) ? 1 : -1;

    // Which way to zoom
    float midpoint = (float)(range+1) / 2.0f;
    if(d <= midpoint)
        depthRange.second -= dir;
    else
        depthRange.first += dir;

    // Constraints
    depthRange.first = max(depthRange.first,0);
    depthRange.second = min(depthRange.second,topo->hardwareResourceMatrix.size());

    // Set width ranges
    widthRange.clear();
    for(int i=depthRange.first; i<depthRange.second; i++)
    {
        IntRange wr(0,topo->hardwareResourceMatrix[i].size());
        widthRange.push_back(wr);
    }
}

void MemTopoViz::zoomHorizontal(hardwareResourceNode *node, int dir)
{
    /*
    int d = node->depth;
    int range = widthRange[d].second - widthRange[d].first;
    if(range <= 1)
        return;

    dir = (dir > 0) ? 1 : -1;

    // Shrink from largest visible node
    for(int i=0; i<widthRange.size(); i++)
    {
        IntRange wr(0,topo->hardwareResourceMatrix[i].size());
        widthRange.first += dir;
        widthRange.second -= dir;
    }
    */
}

void MemTopoViz::wheelEvent(QWheelEvent *e)
{
    if(!processed)
        return;

    /*
    hardwareResourceNode *node = nodeAtPosition(e->pos());

    if(!node)
        return;

    if(e->delta() > 0)
        zoomVertical(node,1);
    else
        zoomVertical(node,-1);

    visibilityChangedSlot();
    */
}

void MemTopoViz::mouseMoveEvent(QMouseEvent* e)
{
    if(!processed)
        return;

    hardwareResourceNode *node = nodeAtPosition(e->pos());

    if(node)
    {
        QString label;

        if(node->depth == dataSet->hwTopo()->totalDepth)
            label = "CPU " + QString::number(node->id) + "\n";
        else if(node->depth > 1)
            label = "L" + QString::number(node->id) + " Cache\n";
        else if(node->depth == 1)
            label = "NUMA Node " + QString::number(node->id) + "\n";
        else
            label = "RAM\n";

        label += "\n";
        label += "Size: " + QString::number(node->size) + " bytes\n";

        /*
        label += "\n";

        int numCycles = node->sampleSets[dataSet->at(0)].second;
        int numSamples = ((SampleIdxVector*)(&node->sampleSets[dataSet->at(0)].first))->size();
        label += "Samples: " + QString::number(numSamples) + "\n";
        label += "Cycles: " + QString::number(numCycles) + "\n";

        label += "\n";
        label += "Cycles/Access: " + QString::number((float)numCycles / (float)numSamples) + "\n";
        */

        QToolTip::showText(e->globalPos(),label,this, rect() );
    }
    else
    {
        QToolTip::hideText();
    }
}

void MemTopoViz::resizeEvent(QResizeEvent *e)
{
    VizWidget::resizeEvent(e);

    if(!processed)
        return;

    resizeNodeBoxes();
    repaint();
}

void MemTopoViz::calcMinMaxes()
{
    RealRange limits;
    limits.first = 99999999;
    limits.second = 0;

    depthValRanges.resize(depthRange.second - depthRange.first);
    depthValRanges.fill(limits);

    for(int r=0, i=depthRange.first; i<depthRange.second; r++, i++)
    {
        // Get min/max for this row
        for(int j=widthRange[r].first; j<widthRange[r].second; j++)
        {
            for(int d=0; d<dataSet->size(); d++)
            {
                hardwareResourceNode *node = dataSet->at(d)->topo->hardwareResourceMatrix[i][j];

                if(!node->sampleSets.contains(dataSet->at(d)))
                    continue;

                SampleIdxVector *samples = &node->sampleSets[dataSet->at(d)].first;
                int *numCycles = &node->sampleSets[dataSet->at(d)].second;

                qreal val = (dataMode == COLORBY_CYCLES) ? *numCycles : samples->size();
                //val = (qreal)(*numCycles) / (qreal)samples->size();

                depthValRanges[i].first=min(depthValRanges[i].first,val);
                depthValRanges[i].second=max(depthValRanges[i].second,val);
            }
        }
    }
}

int compareColoredRect(const void* a, const void* b)
{
    const QRectF *ra = &((ColoredRect*)a)->second;
    const QRectF *rb = &((ColoredRect*)b)->second;

    if(ra->width() > rb->width())
        return -1;
    if(ra->width() == rb->width())
        return 0;
    //if(ra->width() < rb->width())
        return 1;
}

void MemTopoViz::resizeNodeBoxes()
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    nodeBoxes.clear();
    nodeDataBoxes.clear();

    float depth = depthRange.second - depthRange.first;
    float deltaY = drawBox.height() / depth;

    // Adjust boxes to fill the drawBox space
    for(int dy=0, i=depthRange.first; i<depthRange.second; dy++, i++)
    {
        int width = widthRange[dy].second - widthRange[dy].first;
        float maxBoxWidth = drawBox.width() / (float) width;
        int minVal = depthValRanges[i].first;
        int maxVal = depthValRanges[i].second;
        float deltaX = drawBox.width() / (float) width;
        for(int dx=0, j=widthRange[dy].first; j<widthRange[dy].second; dx++, j++)
        {
            // Create node box (no data)
            hardwareResourceNode *node = dataSet->hwTopo()->hardwareResourceMatrix[i][j];
            QRectF box = drawBox;
            box.adjust(deltaX*dx,
                       deltaY*dy,
                       -deltaX*(width-dx-1),
                       -deltaY*(depth-dy-1));
            NodeBox nodeBox(node,box);
            nodeBoxes.push_back(nodeBox);

            // Create sized node boxes for each dataset
            int firstData = nodeDataBoxes.size();
            float boxHeight = box.height() / (float)dataSet->size();
            for(int d=0; d<dataSet->size(); d++)
            {
                QRectF nodeDataBox = box;
                node = dataSet->at(d)->topo->hardwareResourceMatrix[i][j];

                bool cont = node->sampleSets.contains(dataSet->at(d));
                if(!cont)
                    continue;

                SampleIdxVector *samples = &node->sampleSets[dataSet->at(d)].first;
                int *numCycles = &node->sampleSets[dataSet->at(d)].second;

                qreal val = (dataMode == COLORBY_CYCLES) ? *numCycles : samples->size();
                //val = (qreal)(*numCycles) / (qreal)samples->size();
                qreal scaledVal = scale(val,minVal,maxVal,0,1);

                nodeDataBox.setTop(box.top()+d*boxHeight);
                nodeDataBox.setWidth(maxBoxWidth*scaledVal);
                nodeDataBox.setHeight(boxHeight);

                QColor color = valToColor(d,0,dataSet->size(),colorMap);
                ColoredRect nodeDataColoredBox(color,nodeDataBox);
                nodeDataBoxes.push_back(nodeDataColoredBox);
            }

            // Sort smallest rect to largest
            qsort((void*)&nodeDataBoxes[firstData],
                  nodeDataBoxes.size()-firstData,
                  sizeof(ColoredRect),compareColoredRect);
        }
    }
}

hardwareResourceNode *MemTopoViz::nodeAtPosition(QPoint p)
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    for(int b=0; b<nodeBoxes.size(); b++)
    {
        hardwareResourceNode *node = nodeBoxes[b].first;
        QRectF box = nodeBoxes[b].second;

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

void MemTopoViz::selectSamplesWithinNode(hardwareResourceNode *node)
{
    dataSet->selectByResource(node);
    emit selectionChangedSig();
}
