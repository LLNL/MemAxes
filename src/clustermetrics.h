#include "dataobject.h"

#ifndef CLUSTERMETRICS_H
#define CLUSTERMETRICS_H

class ClusterMetric
{
public:
    virtual void createAggregateFromSamples(DataObject *d, ElemSet *s) = 0;
};

class HardwareClusterMetric : public ClusterMetric
{
public:
    virtual void createAggregateFromSamples(DataObject *d, ElemSet *s);

    qreal distance(HardwareClusterMetric *other);
    void createAggregateFromNodes(HardwareClusterMetric *hcm1, HardwareClusterMetric *hcm2);

private:
    hwTopo *topo;

    std::vector<int> depthSamples;
    std::vector<qreal> depthMeans;
    std::vector<qreal> depthStddevs;
};

#endif
