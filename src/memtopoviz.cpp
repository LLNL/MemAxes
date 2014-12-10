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

#include "memtopoviz.h"

#include <iostream>
#include <cmath>

using namespace std;

MemTopoViz::MemTopoViz(QWidget *parent) :
    VizWidget(parent)
{
    dataMode = COLORBY_CYCLES;
    vizMode = SUNBURST;

    colorMap = gradientColorMap(QColor(255,237,160),
                                QColor(240,59 ,32 ),
                                128);

    this->installEventFilter(this);
    setMouseTracking(true);
}

void MemTopoViz::processData()
{
    processed = false;

    if(dataSet->isEmpty() || !dataSet->hwTopo())
        return;


    std::cout << "Hardware Resource Matrix" << std::endl;
    depthRange = IntRange(0,dataSet->hwTopo()->hardwareResourceMatrix.size());
    for(int i=depthRange.first; i<(int)depthRange.second; i++)
    {
        IntRange wr(0,dataSet->hwTopo()->hardwareResourceMatrix[i].size());

        for(int r=0; r<wr.second; r++)
            std::cout << ".";
        std::cout << std::endl;

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

    // Draw node outlines
    painter->setPen(QPen(Qt::black));
    for(int b=0; b<nodeBoxes.size(); b++)
    {
        hardwareResourceNode *node = nodeBoxes[b].node;
        QRectF box = nodeBoxes[b].box;
        QString text = QString::number(node->id);

        // Get value by cycles or samples
        int numCycles = 0;
        int numSamples = 0;
        for(int d=0; d<dataSet->size(); d++)
        {
            numSamples += node->sampleSets[dataSet->at(d)].first.size();
            numCycles += node->sampleSets[dataSet->at(d)].second;
        }

        qreal val = (dataMode == COLORBY_CYCLES) ? numCycles : numSamples;

        // Scale by depth
        int depth = node->depth;
        QColor vcolor = valToColor(val,
                                   depthValRanges[depth].first,
                                   depthValRanges[depth].second,
                                   colorMap);

        // Color by value
        painter->setBrush(vcolor);

        // Draw rect (radial or regular)
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

    // Draw links
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    for(int b=0; b<linkBoxes.size(); b++)
    {
        QRectF box = linkBoxes[b].box;

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

        label += "\n";

        int numCycles = 0;
        int numSamples = 0;
        for(int d=0; d<dataSet->size(); d++)
        {
            numSamples += node->sampleSets[dataSet->at(d)].first.size();
            numCycles += node->sampleSets[dataSet->at(d)].second;
        }

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

    depthTransRanges.resize(depthRange.second - depthRange.first);
    depthTransRanges.fill(limits);

    for(int r=0, i=depthRange.first; i<depthRange.second; r++, i++)
    {
        // Get min/max for this row
        for(int j=widthRange[r].first; j<widthRange[r].second; j++)
        {
            for(int d=0; d<dataSet->size(); d++)
            {
                hardwareResourceNode *node = dataSet->hwTopo()->hardwareResourceMatrix[i][j];

                if(!node->sampleSets.contains(dataSet->at(d)))
                    continue;

                SampleIdxVector *samples = &node->sampleSets[dataSet->at(d)].first;
                int *numCycles = &node->sampleSets[dataSet->at(d)].second;

                qreal val = (dataMode == COLORBY_CYCLES) ? *numCycles : samples->size();
                //val = (qreal)(*numCycles) / (qreal)samples->size();

                depthValRanges[i].first=min(depthValRanges[i].first,val);
                depthValRanges[i].second=max(depthValRanges[i].second,val);

                qreal trans = node->transactions;
                depthTransRanges[i].first=min(depthTransRanges[i].first,trans);
                depthTransRanges[i].second=max(depthTransRanges[i].second,trans);
                
            }
        }
    }
}

void MemTopoViz::resizeNodeBoxes()
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    nodeBoxes.clear();
    linkBoxes.clear();
    nodeDataBoxes.clear();

    float nodeMarginX = 2.0f;
    float nodeMarginY = 10.0f;

    float deltaX = 0;
    float deltaY = drawBox.height() / dataSet->hwTopo()->hardwareResourceMatrix.size();

    // Adjust boxes to fill the drawBox space
    for(int i=0; i<dataSet->hwTopo()->hardwareResourceMatrix.size(); i++)
    {
        deltaX = drawBox.width() / (float)dataSet->hwTopo()->hardwareResourceMatrix[i].size();
        for(int j=0; j<dataSet->hwTopo()->hardwareResourceMatrix[i].size(); j++)
        {
            hardwareResourceNode *node = dataSet->hwTopo()->hardwareResourceMatrix[i][j];
            int depth = node->depth;

            // Create Node Box
            QRectF nodeBox;
            nodeBox.setRect(drawBox.left()+j*deltaX,
                            drawBox.top()+i*deltaY,
                            deltaX,
                            deltaY);

            if(i==0)
                nodeBox.adjust(0,0,0,-nodeMarginY);
            else
                nodeBox.adjust(nodeMarginX,nodeMarginY,-nodeMarginX,-nodeMarginY);

            nodeBoxes.push_back(NodeBox(node,nodeBox));

            // Create Link Box
            if(i-1 >= 0)
            {
                QRectF linkBox;
                linkBox.setRect(drawBox.left()+j*deltaX,
                                drawBox.top()+i*deltaY,
                                deltaX,
                                nodeMarginY);

                linkBox.adjust(nodeMarginX,-nodeMarginY,-nodeMarginX,0);
                
                // scale width by transactions
                float linkWidth = scale(node->transactions,
                                        depthTransRanges[depth].first,
                                        depthTransRanges[depth].second,
                                        1.0f,
                                        linkBox.width());
                float deltaWidth = (linkBox.width()-linkWidth)/2.0f;

                linkBox.adjust(deltaWidth,0,-deltaWidth,0);

                linkBoxes.push_back(LinkBox(node->parent,node,linkBox));
            }
        }
    }
}

hardwareResourceNode *MemTopoViz::nodeAtPosition(QPoint p)
{
    QRectF drawBox = this->rect();
    drawBox.adjust(margin,margin,-margin,-margin);

    for(int b=0; b<nodeBoxes.size(); b++)
    {
        hardwareResourceNode *node = nodeBoxes[b].node;
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

void MemTopoViz::selectSamplesWithinNode(hardwareResourceNode *node)
{
    dataSet->selectByResource(node);
    emit selectionChangedSig();
}
