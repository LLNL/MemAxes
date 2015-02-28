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
    qreal winDelta = 10; // TODO
    qreal threshold = 100; // TODO

    // Create leaf node for every window
    // and an internal node above each leaf
    qreal winMin, winMax;
    std::vector<DataClusterInternalNode*> *levelNodes = new std::vector<DataClusterInternalNode*>();
    for(winMin=0, winMax=winSize; winMax<d->maxAt(dim); winMin+=winDelta, winMax+=winDelta)
    {
        DataClusterLeafNode *newLeaf = new DataClusterLeafNode();
        newLeaf->samples = d->createDimRangeQuery(dim, winMin, winMax);

        DataClusterInternalNode *newNode = new DataClusterInternalNode();
        newNode->metric = new HardwareClusterMetric();
        newNode->metric->createAggregateFromSamples(d,&newLeaf->samples);

        newLeaf->parent = newNode;
        newNode->children.push_back(newLeaf);

        levelNodes->push_back(newNode);
    }

    // Merge up from internal nodes created
    while(levelNodes->size() > 1)
    {
        int numMerges = 0;
        bool merging = false;
        std::vector<DataClusterInternalNode*> *nextLevelNodes = new std::vector<DataClusterInternalNode*>();
        for(size_t i=0; i<levelNodes->size()-1; i++)
        {
            // Merge nodes if below threshold
            DataClusterInternalNode *n1 = levelNodes->at(i);
            DataClusterInternalNode *n2 = levelNodes->at(i+1);

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
                    HardwareClusterMetric *newMetric = (HardwareClusterMetric*)newNode->metric;
                    newMetric->createAggregateFromNodes(m1,m2);

                    n1->parent = newNode;
                    n2->parent = newNode;

                    newNode->children.push_back(n1);
                    newNode->children.push_back(n2);

                    nextLevelNodes->push_back(newNode);
                }
                else
                {
                    // Continuing previous merge group
                    DataClusterInternalNode *mergeNode = nextLevelNodes->back();

                    //n1->parent = mergeNode; //already done
                    n2->parent = mergeNode;

                    //mergeNode->children.push_back(n1); // already done
                    mergeNode->children.push_back(n2);
                }
            }
            else
            {
                // Do not merge
                merging = false;
                nextLevelNodes->push_back(n1);
            }
        }

        delete levelNodes;
        levelNodes = nextLevelNodes;

        // If nothing merged, increase threshold
        if(numMerges == 0)
        {
            threshold *= 2.0;
        }
    }

    delete levelNodes;
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
