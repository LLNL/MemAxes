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

#ifndef HARDWARETOPOLOGY_H
#define HARDWARETOPOLOGY_H

#include <QFile>
#include <QXmlStreamReader>
#include <QMap>

#include <vector>

#include "dataobject.h"

class DataObject;

typedef unsigned long long ElemIndex;
typedef std::set<ElemIndex> ElemSet;

struct SampleSet
{
    int totCycles;
    int selCycles;
    ElemSet totSamples;
    ElemSet selSamples;
};

class hwNode
{
public:
    hwNode();

    QString name;

    int id;
    int depth;
    long long size;
    long long transactions;

    hwNode *parent;
    QVector<hwNode*> children;

    QMap<DataObject*,SampleSet> sampleSets;
};

class hwTopo
{
public:
    hwTopo();

    hwNode *hardwareResourceNodeFromXMLNode(QXmlStreamReader *xml, hwNode *parent);
    int loadHardwareTopologyFromXML(QString fileName);

    QString hardwareName;

    int numCPUs;
    int numNUMADomains;
    int totalDepth;

    hwNode *hardwareResourceRoot;

    QVector<hwNode*> allHardwareResourceNodes;
    QVector< QVector<hwNode*> > hardwareResourceMatrix;

    QVector<hwNode*> *CPUNodes;
    QVector<hwNode*> *NUMANodes;

    QMap<int,hwNode*> CPUIDMap;
    QMap<int,hwNode*> NUMAIDMap;

private:
    void processLoadedTopology();
    void constructHardwareResourceMatrix();
    void addToMatrix(hwNode *node);
};

#endif // HARDWARETOPOLOGY_H
