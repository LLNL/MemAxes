#ifndef DATACLUSTERNODE_H
#define DATACLUSTERNODE_H

#include "dataobject.h"
#include "clustermetrics.h"

class DataClusterTree;
class DataClusterNode;

class DataClusterTree
{
public:
    DataClusterTree();
    ~DataClusterTree();

    void build(DataObject *d, int dim);
    
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

    DataClusterNode *parent;
};
class DataClusterInternalNode : public DataClusterNode
{
public:
    DataClusterInternalNode();
    ~DataClusterInternalNode();

    virtual bool isLeaf() { return false; }
    virtual bool isInternal() { return true; }

    ClusterMetric *metric;
    std::vector<DataClusterNode*> children;
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
