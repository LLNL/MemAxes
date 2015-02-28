#include "clustermetrics.h"

void HardwareClusterMetric::createAggregateFromSamples(DataObject *d, ElemSet *s)
{
    // Copy topology from current dataset
    topo = new hwTopo(d->topo);

    // Collect samples
    topo->collectSamples(d,s);

    // Compute means
    depthSamples.resize(topo->totalDepth+1, 0);
    depthMeans.resize(topo->totalDepth+1, 0);
    depthStddevs.resize(topo->totalDepth+1, 0);

    int cpuDepth = topo->totalDepth;
    ElemSet::iterator it;
    for(it = s->begin(); it != s->end(); it++)
    {
        // Sum up all cycles
        ElemIndex elem = *it;
        int depth = d->at(elem,d->dataSourceDim);
        qreal cycles = d->at(elem,d->latencyDim);

        if(depth == -1)
            continue;

        depthSamples[depth] += depthSamples.at(depth) + 1;
        depthMeans[depth] += depthSamples.at(depth) + cycles;

        depthSamples[cpuDepth] = depthSamples.at(depth) + 1;
        depthMeans[cpuDepth] += depthMeans.at(cpuDepth) + cycles;
    }

    for(int i=0; i<topo->totalDepth; i++)
    {
        // Divide to get mean
        depthMeans[i] = depthMeans.at(i) / (qreal)depthSamples.at(i);
    }

    // Compute standard deviations
    for(it = s->begin(); it != s->end(); it++)
    {
        // Sum up squared difference btwn sample and mean
        ElemIndex elem = *it;
        int depth = d->at(elem,d->dataSourceDim);
        qreal cycles = d->at(elem,d->latencyDim);

        if(depth == -1)
            continue;

        depthStddevs[depth] = depthStddevs.at(depth)
                + (cycles-depthMeans.at(depth))*(cycles-depthMeans.at(depth));
        depthStddevs[cpuDepth] = depthStddevs.at(cpuDepth)
                + (cycles-depthMeans.at(cpuDepth))*(cycles-depthMeans.at(cpuDepth));
    }
    for(int i=0; i<topo->totalDepth; i++)
    {
        // Divide to get standard deviation
        depthStddevs[i] = depthStddevs.at(i) / (qreal)depthSamples.at(i);
    }

}

qreal HardwareClusterMetric::distance(HardwareClusterMetric *other)
{
    // Assume same topology (for now)

    // Euclidean length of means vector
    qreal dist = 0;
    for(int i=0; i<topo->totalDepth; i++)
    {
        dist += (depthMeans.at(i) - other->depthMeans.at(i))
                *(depthMeans.at(i) - other->depthMeans.at(i));

    }
    dist = sqrt(dist);

    return dist;
}

void HardwareClusterMetric::createAggregateFromNodes(HardwareClusterMetric *hcm1, HardwareClusterMetric *hcm2)
{
    topo = new hwTopo(hcm1->topo);
    for(int i=0; i<topo->hardwareResourceMatrix.size(); i++)
    {
        for(int j=0; j<topo->hardwareResourceMatrix.at(i).size(); j++)
        {
            hwNode *n = topo->hardwareResourceMatrix.at(i).at(j);
            hwNode *n1 = hcm1->topo->hardwareResourceMatrix.at(i).at(j);
            hwNode *n2 = hcm2->topo->hardwareResourceMatrix.at(i).at(j);

            std::set_union(n1->allSamples.begin(),n1->allSamples.end(),
                           n2->allSamples.begin(),n2->allSamples.end(),
                           std::inserter(n->allSamples,n->allSamples.begin()));
            std::set_union(n1->selectedSamples.begin(),n1->selectedSamples.end(),
                           n2->selectedSamples.begin(),n2->selectedSamples.end(),
                           std::inserter(n->selectedSamples,n->selectedSamples.begin()));

            n->numAllCycles = n1->numAllCycles + n2->numAllCycles;
            n->numSelectedCycles = n1->numSelectedCycles + n2->numSelectedCycles;
            n->numTransactions = n1->numTransactions + n2->numTransactions;
        }
    }

    for(size_t i=0; i<depthSamples.size(); i++)
    {
        depthSamples[i] = hcm1->depthSamples.at(i) + hcm2->depthSamples.at(i);
        depthMeans[i] = (hcm1->depthSamples.at(i)*hcm1->depthMeans.at(i)
                         + hcm2->depthSamples.at(i)*hcm2->depthMeans.at(i))
                        / depthSamples.at(i);
        depthStddevs[i] = (hcm1->depthSamples.at(i)*hcm1->depthStddevs.at(i)
                         + hcm2->depthSamples.at(i)*hcm2->depthStddevs.at(i))
                        / depthSamples.at(i);
    }

}
