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

#include "hwtopodrawable.h"

#include <QPainter>

HWTopoPainter::HWTopoPainter(HWTopo *t)
{
    topo = t;
    rect = QRectF(0,0,10,10);

    dataMode = COLORBY_CYCLES;
    vizMode = SUNBURST;
    colorMap = gradientColorMap(QColor(255,237,160),QColor(240,59 ,32), 256);
}

HWTopoPainter::~HWTopoPainter()
{

}

void HWTopoPainter::calcMinMaxes()
{
    if(topo == NULL)
        return;

    RealRange limits;
    limits.first = 99999999;
    limits.second = 0;

    depthValRanges.resize(topo->totalDepth+1);
    depthValRanges.fill(limits);

    depthTransRanges.resize(topo->totalDepth+1);
    depthTransRanges.fill(limits);

    for(int r=0; r<topo->totalDepth+1; r++)
    {
        std::vector<HWNode*> *depthNodes = &topo->hardwareResourceMatrix.at(r);
        for(unsigned int j=0; j<depthNodes->size(); j++)
        {
            HWNode *node = depthNodes->at(j);

            qreal val = (dataMode == COLORBY_CYCLES) ?
                        node->numSelectedCycles
                        : node->selectedSamples.size();

            depthValRanges[r].first=0;
            depthValRanges[r].second=std::max(depthValRanges[r].second,val);

            qreal trans = node->numTransactions;
            depthTransRanges[r].first=0;
            depthTransRanges[r].second=std::max(depthTransRanges[r].second,trans);
        }
    }
}

void HWTopoPainter::resize(QRectF r)
{
    if(topo == NULL)
        return;

    rect = r;

    nodeBoxes.clear();
    linkBoxes.clear();

    if(topo == NULL)
        return;

    float nodeMarginX = 2.0f;
    float nodeMarginY = 10.0f;

    float deltaX = 0;
    float deltaY = rect.height() / topo->hardwareResourceMatrix.size();

    // Adjust boxes to fill the rect space
    for(unsigned int i=0; i<topo->hardwareResourceMatrix.size(); i++)
    {
        deltaX = rect.width() / (float)topo->hardwareResourceMatrix[i].size();
        for(unsigned int j=0; j<topo->hardwareResourceMatrix[i].size(); j++)
        {
            // Create Node Box
            NodeBox nb;
            nb.node = topo->hardwareResourceMatrix[i][j];
            nb.box.setRect(rect.left()+j*deltaX,
                           rect.top()+i*deltaY,
                           deltaX,
                           deltaY);

            // Get value by cycles or samples
            int numCycles = nb.node->numSelectedCycles;
            int numSamples = nb.node->selectedSamples.size();

            qreal unscaledval = (dataMode == COLORBY_CYCLES) ? numCycles : numSamples;
            nb.val = scale(unscaledval,
                           depthValRanges.at(i).first,
                           depthValRanges.at(i).second,
                           0, 1);

            if(i==0)
                nb.box.adjust(0,0,0,-nodeMarginY);
            else
                nb.box.adjust(nodeMarginX,nodeMarginY,-nodeMarginX,-nodeMarginY);

            nodeBoxes.push_back(nb);

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

                float linkWidth = scale(nb.node->numTransactions,
                                        depthTransRanges.at(i).first,
                                        depthTransRanges.at(i).second,
                                        1.0f,
                                        lb.box.width());
                float deltaWidth = (lb.box.width()-linkWidth)/2.0f;

                lb.box.adjust(deltaWidth,0,-deltaWidth,0);

                linkBoxes.push_back(lb);
            }
        }
    }
}


void HWTopoPainter::draw(QPainter *painter)
{
    if(topo == NULL)
        return;

    // Draw node outlines
    painter->setPen(QPen(Qt::black));
    for(int b=0; b<nodeBoxes.size(); b++)
    {
        HWNode *node = nodeBoxes.at(b).node;
        QRectF box = nodeBoxes.at(b).box;
        QString text = QString::number(node->id);

        // Color by value
        QColor col = valToColor(nodeBoxes.at(b).val,colorMap);
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
    for(int b=0; b<linkBoxes.size(); b++)
    {
        QRectF box = linkBoxes.at(b).box;

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

HWNode *HWTopoPainter::nodeAtPosition(QPoint p)
{
    if(topo == NULL)
        return NULL;

    for(int b=0; b<nodeBoxes.size(); b++)
    {
        HWNode *node = nodeBoxes.at(b).node;
        QRectF box = nodeBoxes.at(b).box;

        bool containsP = false;
        if(vizMode == SUNBURST)
        {
            QPointF radp = reverseRadialTransform(p,rect);
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

