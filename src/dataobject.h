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

#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QWidget>
#include <QBitArray>

#include <map>
#include <set>
#include <vector>
#include <assert.h>

#include "hwtopo.h"
#include "util.h"
#include "console.h"

#define INVISIBLE false
#define VISIBLE true

class hwTopo;
class hwNode;
class console;

typedef unsigned long long ElemIndex;
typedef std::set<ElemIndex> ElemSet;

enum selection_mode
{
    MODE_NEW = 0,
    MODE_APPEND,
    MODE_FILTER
};

struct indexedValue
{
    ElemIndex idx;
    qreal val;

    bool operator<(const struct indexedValue &other) const
        { return val < other.val; }
    bool operator>(const struct indexedValue &other) const
        { return val > other.val; }
};

typedef std::vector<indexedValue> IndexList;

// Distance Functions (for clustering)
typedef qreal (*distance_metric_fn_t)(DataObject *d, ElemSet *s1, ElemSet *s2);
qreal distanceHardware(DataObject *d, ElemSet *s1, ElemSet *s2);

class DataObject
{
public:
    DataObject();


    hwTopo *getTopo() { return topo; }
    int loadHardwareTopology(QString filename);
    bool empty() { return numElements == 0; }

    // Initialization
    int loadData(QString filename);
    void selectionChanged() { collectTopoSamples(); }
    void visibilityChanged() { collectTopoSamples(); }

    void setConsole(console *c) { con = c; }

private:
    void allocate();
    void collectTopoSamples();
    int parseCSVFile(QString dataFileName);

public:
    // Selection & Visibility
    selection_mode selectionMode() { return selMode; }
    void setSelectionMode(selection_mode mode, bool silent = false);
    int selected(ElemIndex index);
    bool visible(ElemIndex index);
    bool selectionDefined();

    void selectData(ElemIndex index, int group = 1);
    void selectAll(int group = 1);
    void deselectAll();
    void selectAllVisible(int group = 1);

    void showData(unsigned int index);
    void hideData(unsigned int index);
    void showAll();
    void hideAll();
    void hideSelected();
    void hideUnselected();

    void selectSet(ElemSet &s, int group = 1);
    void selectByDimRange(int dim, qreal vmin, qreal vmax, int group = 1);
    void selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes, int group = 1);
    void selectBySourceFileName(QString str, int group = 1);
    void selectByVarName(QString str, int group = 1);
    void selectByResource(hwNode *node, int group = 1);

    ElemSet& getSelectionSet(int group = 1) { return selectionSets.at(group); }

    // Calculated statistics
    void calcStatistics();
    void constructSortedLists();

    qreal at(int i, int d) const { return vals[i*numDimensions+d]; }
    qreal sumAt(int d) const { return dimSums[d]; }
    qreal minAt(int d) const { return minimumValues[d]; }
    qreal maxAt(int d) const { return maximumValues[d]; }
    qreal meanAt(int d) const { return meanValues[d]; }
    qreal stddevAt(int d) const { return standardDeviations[d]; }
    qreal covarianceBtwn(int d1,int d2) const 
        { return covarianceMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }
    qreal correlationBtwn(int d1,int d2) const 
        { return correlationMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }

    // Hierarchical clustering
    void cluster(distance_metric_fn_t dfn);

public:
    QStringList meta;
    hwTopo *topo;

    // Counts
    ElemIndex numDimensions;
    ElemIndex numElements;
    ElemIndex numSelected;
    ElemIndex numVisible;

    // Hard-coded dimensions
    int sourceDim;
    int lineDim;
    int variableDim;
    int dataSourceDim;
    int indexDim;
    int latencyDim;
    int cpuDim;
    int nodeDim;
    int xDim;
    int yDim;
    int zDim;

    QVector<qreal> vals;
    QVector<QString> fileNames;
    QVector<QString> varNames;
    QVector<qreal>::Iterator begin;
    QVector<qreal>::Iterator end;

private:
    QBitArray visibility;
    QVector<int> selectionGroup;
    std::vector<ElemSet> selectionSets;

    std::vector<IndexList> dimSortedLists;

    QVector<qreal> dimSums;
    QVector<qreal> minimumValues;
    QVector<qreal> maximumValues;
    QVector<qreal> meanValues;
    QVector<qreal> standardDeviations;
    QVector<qreal> covarianceMatrix;
    QVector<qreal> correlationMatrix;

private:
    console *con;
    QVector<DataObject*> dataObjects;

    int selGroup;
    selection_mode selMode;
};

#endif // DATAOBJECT_H
