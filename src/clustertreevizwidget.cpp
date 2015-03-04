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

#include "clustertreevizwidget.h"

ClusterTreeVizWidget::ClusterTreeVizWidget(QWidget *parent)
{
    needsResize = false;
    needsRepaint = false;
    needsGatherPainters = false;
    clusterIndex = 0;
    clusterDepth = 3;
}

std::vector<DataClusterNode*>
ClusterTreeVizWidget::getNodesAtDepth(DataClusterNode *root, int depth)
{
    std::vector<DataClusterNode*> nodes;
    if(root->isLeaf())
    {
        return nodes; // don't add leaves
    }

    if(depth == 0)
    {
        nodes.push_back(root);
        return nodes;
    }

    // root is internal, we can cast
    DataClusterInternalNode *inode = (DataClusterInternalNode*)root;
    for(unsigned int i=0; i<inode->children.size(); i++)
    {
        std::vector<DataClusterNode*> chnodes;
        chnodes = getNodesAtDepth(inode->children.at(i),depth-1);

        nodes.insert(nodes.end(),chnodes.begin(),chnodes.end());
    }

    return nodes;
}

void ClusterTreeVizWidget::gatherPainters()
{
    currentHwTopoPainters.clear();

    DataClusterTree *tree = dataSet->clusterTrees.at(clusterIndex);

    std::vector<DataClusterNode*> depthNodes = getNodesAtDepth(tree->getRoot(),clusterDepth);

    for(unsigned int i=0; i<depthNodes.size(); i++)
    {
        // getNodesAtDepth only returns internal nodes
        DataClusterInternalNode *inode = (DataClusterInternalNode*)depthNodes.at(i);

        // we're assuming hardware cluster metrics (for drawing aggregates)
        HardwareClusterMetric *met = (HardwareClusterMetric*)inode->metric;
        HWTopoPainter htp(met->getTopo());

        // prepare
        htp.calcMinMaxes();
        currentHwTopoPainters.push_back(htp);
    }
}

void ClusterTreeVizWidget::frameUpdate()
{
    if(needsGatherPainters)
    {
        gatherPainters();
        needsResize = true;
        needsGatherPainters = false;
    }
    if(needsResize)
    {
        int topoSize = 250;
        int totalHeight = 0;
        QRectF topoBox(0,0,topoSize,topoSize);
        for(unsigned int i=0; i<currentHwTopoPainters.size(); i++)
        {
            currentHwTopoPainters.at(i).resize(topoBox);
            topoBox.moveCenter(topoBox.center()+QPointF(0,topoSize));
            totalHeight += topoSize;
        }
        needsRepaint = true;
        needsResize = false;
        this->setFixedHeight(totalHeight);
    }
    if(needsRepaint)
    {
        repaint();
        needsRepaint = false;
    }
}

void ClusterTreeVizWidget::drawQtPainter(QPainter *painter)
{
    for(unsigned int i=0; i<currentHwTopoPainters.size(); i++)
    {
        currentHwTopoPainters.at(i).draw(painter);
    }
}

