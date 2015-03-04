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

#include "datacluster.h"

DataClusterTree::DataClusterTree()
{
}

DataClusterTree::~DataClusterTree()
{
}

void DataClusterTree::build(DataObject *d, int dim)
{
    qreal winSize = 100; // TODO
    qreal winDelta = 50; // TODO
    qreal threshold = 1; // TODO

    // Create leaf node for every window
    // and an internal node above each leaf
    qreal winMin, winMax;
    std::vector<DataClusterInternalNode*> levelNodes;
    for(winMin=0, winMax=winSize; winMax<d->maxAt(dim); winMin+=winDelta, winMax+=winDelta)
    {
        DataClusterLeafNode *newLeaf = new DataClusterLeafNode();
        newLeaf->samples = d->createDimRangeQuery(dim, winMin, winMax);

        DataClusterInternalNode *newNode = new DataClusterInternalNode();
        newNode->metric = new HardwareClusterMetric();
        newNode->metric->createAggregateFromSamples(d,&newLeaf->samples);

        newLeaf->parent = newNode;
        newNode->children.push_back(newLeaf);

        levelNodes.push_back(newNode);
    }

    // Merge up from internal nodes created
    std::vector<DataClusterInternalNode*> nextLevelNodes;
    while(levelNodes.size() > 1)
    {
        int numMerges = 0;
        bool merging = false;
        for(size_t i=0; i<levelNodes.size()-1; i++)
        {
            // Merge nodes if below threshold
            DataClusterInternalNode *n1 = levelNodes.at(i);
            DataClusterInternalNode *n2 = levelNodes.at(i+1);

            HardwareClusterMetric* m1 = (HardwareClusterMetric*)n1->metric;
            HardwareClusterMetric* m2 = (HardwareClusterMetric*)n2->metric;

            // Run distance metric
            qreal dist = m1->distance(m2);

            if(dist < threshold)
            {
                numMerges++;

                if(!merging)
                {
                    // New merged group
                    merging = true;
                    DataClusterInternalNode *newNode = new DataClusterInternalNode();

                    newNode->metric = new HardwareClusterMetric();

                    HardwareClusterMetric *newMetric = (HardwareClusterMetric*)newNode->metric;
                    newMetric->setTopo(new HWTopo(m1->getTopo())); // copy topo from m1
                    newMetric->createAggregateFromNodes(m1,m2); // create aggregate

                    n1->parent = newNode;
                    n2->parent = newNode;

                    newNode->children.push_back(n1);
                    newNode->children.push_back(n2);

                    nextLevelNodes.push_back(newNode);
                }
                else
                {
                    // Continuing previous merge group
                    DataClusterInternalNode *mergeNode = nextLevelNodes.back();

                    //n1->parent = mergeNode; //already done
                    n2->parent = mergeNode;

                    // BIG TODO:
                    // add aggregate of n2 into mergeNode's aggregate

                    //mergeNode->children.push_back(n1); // already done
                    mergeNode->children.push_back(n2);
                }
            }
            else
            {
                if(!merging)
                    nextLevelNodes.push_back(n1);
                else
                    merging = false;
            }
        }

        // If nothing merged, increase threshold
        if(numMerges == 0)
        {
            threshold *= 2.0;
        }
        else
        {
            std::swap(levelNodes,nextLevelNodes);
        }
        nextLevelNodes.clear();
    }

    root = levelNodes.back();
}

DataClusterNode::DataClusterNode()
{
}

DataClusterNode::~DataClusterNode()
{
}

DataClusterInternalNode::DataClusterInternalNode()
{
}

DataClusterInternalNode::~DataClusterInternalNode()
{
}

DataClusterLeafNode::DataClusterLeafNode()
{
}

DataClusterLeafNode::~DataClusterLeafNode()
{
}
