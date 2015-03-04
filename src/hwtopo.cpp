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

#include "hwtopo.h"

#include <iostream>
using namespace std;

HWNode::HWNode()
{
}

HWNode::HWNode(HWNode *other, HWNode *p)
{
    // new root
    parent = p;

    // copy everything else
    name = other->name;
    id = other->id;
    depth = other->depth;
    size = other->size;

    numSelectedCycles = other->numSelectedCycles;
    numAllCycles = other->numAllCycles;
    numTransactions = other->numTransactions;

    std::copy(other->allSamples.begin(),other->allSamples.end(),
              std::inserter(allSamples,allSamples.begin()));
    std::copy(other->selectedSamples.begin(),other->selectedSamples.end(),
              std::inserter(selectedSamples,selectedSamples.begin()));

    for(unsigned int i=0; i<other->children.size(); i++)
    {
        // children root to this
        HWNode *newChild = new HWNode(other->children.at(i),this);
        children.push_back(newChild);
    }
}

HWTopo::HWTopo()
{
    numCPUs = 0;
    numNUMADomains = 0;
    totalDepth = 0;

    hardwareResourceRoot = NULL;
}

HWTopo::HWTopo(HWTopo *other)
{
    numCPUs = other->numCPUs;
    numNUMADomains = other->numNUMADomains;
    totalDepth = other->totalDepth;

    // Copy using hwNode copy
    hardwareResourceRoot = new HWNode(other->hardwareResourceRoot,NULL);

    processLoadedTopology();
}

HWTopo::~HWTopo()
{
    hardwareResourceRoot = NULL;

    for(unsigned int i=0; i<allHardwareResourceNodes.size(); i++)
    {
        delete allHardwareResourceNodes.at(i);
    }
}

HWNode *HWTopo::hardwareResourceNodeFromXMLNode(QXmlStreamReader *xml, HWNode *parent)
{
    HWNode *newLevel = new HWNode();

    newLevel->parent = parent;
    newLevel->name = xml->name().toString(); // Hardware, NUMA, Cache, CPU
    newLevel->depth = (parent == NULL) ? 0 : parent->depth + 1;

    totalDepth = max(totalDepth, newLevel->depth);

    if(xml->attributes().hasAttribute("id"))
        newLevel->id = xml->attributes().value("id").toString().toLongLong();

    if(xml->attributes().hasAttribute("size"))
        newLevel->size = xml->attributes().value("size").toString().toLongLong();

    // Read and add children if existent
    xml->readNext();

    while(!(xml->isEndElement() && xml->name() == newLevel->name))
    {
        if(xml->isStartElement())
        {
            HWNode *child = hardwareResourceNodeFromXMLNode(xml, newLevel);
            newLevel->children.push_back(child);
        }

        xml->readNext();
    }

    return newLevel;
}

int HWTopo::loadHardwareTopologyFromXML(QString fileName)
{
    QFile* file = new QFile(fileName);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QXmlStreamReader xml(file);
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if(xml.isStartDocument())
            continue;

        if(xml.isStartElement())
            if(xml.name() == "Hardware")
                hardwareResourceRoot = hardwareResourceNodeFromXMLNode(&xml,NULL);
    }

    if(xml.hasError()) {
        return -1;
    }

    file->close();
    xml.clear();

    processLoadedTopology();

    return 0;
}

void HWTopo::processLoadedTopology()
{
    allHardwareResourceNodes.clear();
    hardwareResourceMatrix.clear();

    CPUNodes = NULL;
    NUMANodes = NULL;

    CPUIDMap.clear();
    NUMAIDMap.clear();

    constructHardwareResourceMatrix();

    NUMANodes = &hardwareResourceMatrix[1];
    CPUNodes = &hardwareResourceMatrix[totalDepth];

    numNUMADomains = NUMANodes->size();
    numCPUs = CPUNodes->size();

    // Map NUMA IDs to NUMA Nodes
    for(unsigned int i=0; i<NUMANodes->size(); i++)
        NUMAIDMap[NUMANodes->at(i)->id] = NUMANodes->at(i);

    // Map CPU IDs to CPU Nodes
    for(unsigned int i=0; i<CPUNodes->size(); i++)
        CPUIDMap[CPUNodes->at(i)->id] = CPUNodes->at(i);

}

void HWTopo::addToMatrix(HWNode *node)
{
    hardwareResourceMatrix[node->depth].push_back(node);
    allHardwareResourceNodes.push_back(node);

    if(node->children.empty())
        return;

    for(unsigned int i=0; i<node->children.size(); i++)
    {
        addToMatrix(node->children[i]);
    }

}

void HWTopo::constructHardwareResourceMatrix()
{
   hardwareResourceMatrix.resize(totalDepth+1);
   addToMatrix(hardwareResourceRoot);
}

void HWTopo::collectSamples(DataObject *d, ElemSet *s)
{
    // Reset info
    for(unsigned int i=0; i<allHardwareResourceNodes.size(); i++)
    {
        allHardwareResourceNodes[i]->numTransactions = 0;
        allHardwareResourceNodes[i]->numAllCycles = 0;
        allHardwareResourceNodes[i]->numSelectedCycles = 0;
        allHardwareResourceNodes[i]->allSamples.clear();
        allHardwareResourceNodes[i]->selectedSamples.clear();
    }

    // Go through each sample and add it to the right topo node
    bool selDef = d->selectionDefined();
    ElemSet::iterator it;
    for(it = s->begin(); it != s->end(); it++)
    {
        ElemIndex elem = *it;
        bool tsel = !selDef || d->selected(elem);

        // Get vars
        int dse = (int)d->at(elem,d->dataSourceDim);
        int cpu = (int)d->at(elem,d->cpuDim);
        int lat = (int)d->at(elem,d->latencyDim);

        // Search for nodes
        HWNode *cpuNode = CPUIDMap[cpu];
        HWNode *node = cpuNode;

        // Update data for serving resource
        node->allSamples.insert(elem);
        node->numAllCycles += lat;
        if(tsel)
        {
            node->selectedSamples.insert(elem);
            node->numSelectedCycles += lat;
        }

        if(dse == -1)
            continue;

        // Go up to data source
        for( /*init*/; dse>0 && node->parent; dse--, node=node->parent)
        {
            if(tsel)
            {
                node->numTransactions++;
            }
        }

        // Update data for core
        node->allSamples.insert(elem);
        node->numAllCycles += lat;
        if(tsel)
        {
            node->selectedSamples.insert(elem);
            node->numSelectedCycles += lat;
        }
    }
}

ElemSet HWTopo::getAllSamples()
{
    ElemSet allSamples;

    for(unsigned int i=0; i<allHardwareResourceNodes.size(); i++)
    {
        allSamples.insert(allHardwareResourceNodes.at(i)->allSamples.begin(),
                          allHardwareResourceNodes.at(i)->allSamples.end());
    }

    return allSamples;
}
