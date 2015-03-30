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

#include <limits>
#include <cmath>

HardwareClusterAggregate::HardwareClusterAggregate()
{
}

void HardwareClusterAggregate::createAggregateFromSamples(DataObject *d, ElemSet *s)
{
    // Copy topology from current dataset
    setTopo(new HWTopo(d->topo));

    // Set samples to provided ElemSet s
    topo->collectSamples(d,s);

    // Compute means
    depthSamples.resize(topo->totalDepth+1, 0);
    depthAvgSamples.resize(topo->totalDepth+1, 0);
    depthMeans.resize(topo->totalDepth+1, 0);
    depthStddevs.resize(topo->totalDepth+1, 0);
    depthImbalances.resize(topo->totalDepth+1, 0);

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
        depthStddevs[i] = sqrt(depthStddevs.at(i) / (qreal)depthSamples.at(i));
    }

    // Compute imbalance
    for(unsigned int i=0; i<topo->hardwareResourceMatrix.size(); i++)
    {
        qreal minsam = std::numeric_limits<qreal>::max();
        qreal maxsam = std::numeric_limits<qreal>::min();

        qreal numres = topo->hardwareResourceMatrix.at(i).size();
        depthAvgSamples[i] = depthSamples.at(i) / numres;
        for(unsigned int r=0; r<numres; r++)
        {
            qreal rd_i = (qreal)topo->hardwareResourceMatrix.at(i).at(r)->allSamples.size();
            qreal m_i = depthAvgSamples[i];
            qreal sqddiff = (rd_i-m_i)*(rd_i-m_i);
            maxsam = std::max(sqddiff,maxsam);
            minsam = std::min(sqddiff,minsam);
        }

        depthImbalances[i] = maxsam / depthStddevs.at(i);
    }

}

qreal HardwareClusterAggregate::distance(HardwareClusterAggregate *other, METRIC_TYPE m)
{
    return std::abs(getMetric(m)-other->getMetric(m));
}

void HardwareClusterAggregate::initFrom(DataObject *d, HardwareClusterAggregate *hcm)
{
    // Copy topo and set this to it
    this->setTopo(new HWTopo(hcm->getTopo()));
}

void HardwareClusterAggregate::combineAggregate(DataObject *d, HardwareClusterAggregate *hcm)
{
    ElemSet thisSamples = topo->getAllSamples();
    ElemSet hcmSamples = hcm->getTopo()->getAllSamples();

    thisSamples.insert(hcmSamples.begin(),hcmSamples.end());

    createAggregateFromSamples(d,&thisSamples);
}

void HardwareClusterAggregate::setTopo(HWTopo *t)
{
    topo = t;
    depthSamples.resize(topo->totalDepth+1, 0);
    depthAvgSamples.resize(topo->totalDepth+1, 0);
    depthMeans.resize(topo->totalDepth+1, 0);
    depthStddevs.resize(topo->totalDepth+1, 0);
    depthImbalances.resize(topo->totalDepth+1, 0);
}

qreal HardwareClusterAggregate::getMetric(METRIC_TYPE t)
{
    switch(t)
    {
        case(CORE_IMBALANCE):
            return getCoreImbalance();
        case(L1_IMBALANCE):
            return getL1Imbalance();
        case(L2_IMBALANCE):
            return getL2Imbalance();
        case(L3_IMBALANCE):
            return getL3Imbalance();
        case(NUMA_IMBALANCE):
            return getNUMAImbalance();
        default:
            return -1;
    }
}

qreal HardwareClusterAggregate::getCoreImbalance()
{
    return depthImbalances.back();
}

qreal HardwareClusterAggregate::getL1Imbalance()
{
    return depthImbalances.at(depthImbalances.size()-2);
}

qreal HardwareClusterAggregate::getL2Imbalance()
{
    return depthImbalances.at(depthImbalances.size()-3);
}

qreal HardwareClusterAggregate::getL3Imbalance()
{

    return depthImbalances.at(depthImbalances.size()-4);
}

qreal HardwareClusterAggregate::getNUMAImbalance()
{
    return depthImbalances.at(depthImbalances.size()-5);
}
