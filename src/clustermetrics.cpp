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

#include "clustermetrics.h"

void HardwareClusterMetric::createAggregateFromSamples(DataObject *d, ElemSet *s)
{
    // Copy topology from current dataset
    topo = new HWTopo(d->topo);

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

        depthSamples[depth] = depthSamples.at(depth) + 1;
        depthMeans[depth] = depthSamples.at(depth) + cycles;

        depthSamples[cpuDepth] = depthSamples.at(depth) + 1;
        depthMeans[cpuDepth] = depthMeans.at(cpuDepth) + cycles;
    }

    for(int i=0; i<topo->totalDepth; i++)
    {
        // Divide to get mean
        if(depthSamples.at(i) == 0) // div by zero
            depthMeans[i] = 0;

        else
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

void HardwareClusterMetric::initFrom(HardwareClusterMetric *hcm)
{
    // Copy topo and set this to it
    this->setTopo(new HWTopo(hcm->getTopo()));
}

void HardwareClusterMetric::combineAggregate(HardwareClusterMetric *hcm)
{
    for(unsigned int i=0; i<topo->hardwareResourceMatrix.size(); i++)
    {
        for(unsigned int j=0; j<topo->hardwareResourceMatrix.at(i).size(); j++)
        {
            ElemSet tmpSet;

            HWNode *n = topo->hardwareResourceMatrix.at(i).at(j);
            HWNode *n1 = hcm->topo->hardwareResourceMatrix.at(i).at(j);

            std::set_union(n->allSamples.begin(),n->allSamples.end(),
                           n1->allSamples.begin(),n1->allSamples.end(),
                           std::inserter(tmpSet,tmpSet.begin()));

            std::swap(n->allSamples, tmpSet);
            tmpSet.clear();

            std::set_union(n->selectedSamples.begin(),n->selectedSamples.end(),
                           n1->selectedSamples.begin(),n1->selectedSamples.end(),
                           std::inserter(tmpSet,tmpSet.begin()));

            std::swap(n->selectedSamples, tmpSet);

            n->numAllCycles = n->numAllCycles + n1->numAllCycles;
            n->numSelectedCycles = n->numSelectedCycles + n1->numSelectedCycles;
            n->numTransactions = n->numTransactions + n1->numTransactions;
        }
    }

    for(unsigned int i=0; i<depthSamples.size(); i++)
    {
        int allDepthSamples = depthSamples.at(i) + hcm->depthSamples.at(i);
        depthMeans[i] = (depthSamples.at(i)*depthMeans.at(i)
                         + hcm->depthSamples.at(i)*hcm->depthMeans.at(i))
                         / std::max(allDepthSamples,1); // div by zero
        depthStddevs[i] = (hcm->depthSamples.at(i)*hcm->depthStddevs.at(i)
                         + hcm->depthSamples.at(i)*hcm->depthStddevs.at(i))
                         / std::max(allDepthSamples,1); // div by zero
        depthSamples[i] = allDepthSamples;
    }
}

void HardwareClusterMetric::setTopo(HWTopo *t)
{
    topo = t;
    depthSamples.resize(topo->totalDepth+1, 0);
    depthMeans.resize(topo->totalDepth+1, 0);
    depthStddevs.resize(topo->totalDepth+1, 0);
}
