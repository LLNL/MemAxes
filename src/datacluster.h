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

#ifndef DATACLUSTERNODE_H
#define DATACLUSTERNODE_H

#include "dataobject.h"
#include "clustermetrics.h"

class ClusterAggregate;

class DataClusterTree;
class DataClusterNode;
class DataClusterInternalNode;
class DataClusterLeafNode;

class DataClusterTree
{
public:
    DataClusterTree();
    ~DataClusterTree();

    void build(DataObject *d, int dim);
    DataClusterNode* getRoot() { return root; }
    std::vector<DataClusterNode*> getNodesAtDepth(int depth);

private:
    std::vector<DataClusterLeafNode*> createUniformWindowLeaves(DataObject *d, int dim, int overlap, int targetLeaves);
    std::vector<DataClusterLeafNode*> createEqualSizedLeaves(DataObject *d, int dim, int targetLeaves);
    std::vector<DataClusterInternalNode *> createInternalFromLeaves(DataObject *d, std::vector<DataClusterLeafNode *> &leafNodes);

    void hierarchicalCluster(DataObject *d, std::vector<DataClusterLeafNode*> &leafNodes);
    
private:
    DataClusterNode *root;
};

class DataClusterNode
{
public:
    DataClusterNode();
    ~DataClusterNode();

    virtual bool isInternal() = 0;
    virtual bool isLeaf() = 0;

    void setRange(qreal mi, qreal ma) { rangeMin = mi; rangeMax = ma; }

    std::vector<DataClusterNode*> getNodesAtDepth(int depth);

    qreal rangeMin;
    qreal rangeMax;

    DataClusterNode *parent;
    std::vector<DataClusterNode*> children;
};
class DataClusterInternalNode : public DataClusterNode
{
public:
    DataClusterInternalNode();
    ~DataClusterInternalNode();

    virtual bool isLeaf() { return false; }
    virtual bool isInternal() { return true; }

    ClusterAggregate *aggregate;
};
class DataClusterLeafNode : public DataClusterNode
{
public:
    DataClusterLeafNode();
    ~DataClusterLeafNode();

    virtual bool isLeaf() { return true; }
    virtual bool isInternal() { return false; }

    ElemSet samples;
};

#endif
