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

#include "hardwaretopology.h"
#include "util.h"
#include "console.h"

#define INVISIBLE false
#define VISIBLE true

class DataSetObject;
class hardwareTopology;
class hardwareResourceNode;
class console;

enum selection_mode
{
    MODE_NEW = 0,
    MODE_APPEND,
    MODE_FILTER
};

class DataObject
{
    friend class DataSetObject;

public:
    DataObject();

public:
    // Initialization
    void allocate();
    void collectTopoSamples(hardwareTopology *hw, bool sel = false);
    int parseCSVFile(QString dataFileName);
    void selectionChanged() { collectTopoSamples(topo); }

    // Selection
    int selected(unsigned int index);
    bool visible(unsigned int index);

    bool selectionDefined();
    bool skip(unsigned int index);

    void selectData(unsigned int index, int group = -1);
    void deselectData(unsigned int index);
    void logicalSelectData(unsigned int index, bool select, int group = -1);

    void selectAll();
    void deselectAll();
    void selectAllVisible();

    // Visibility
    void showData(unsigned int index);
    void hideData(unsigned int index);
    void showAll();
    void hideAll();
    void hideSelected();
    void hideUnselected();

    // Selection Queries
    void selectByDimRange(int dim, qreal vmin, qreal vmax);
    void selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes);
    void selectBySourceFileName(QString str);
    void selectByVarName(QString str);
    void selectByResource(hardwareResourceNode *node);

    // Calculated statistics
    void calcStatistics(int group = 0);

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

    // Distance Functions (for clustering)
    qreal distanceHardware(DataObject *dso);

public:
    QStringList meta;
    hardwareTopology *topo;

    // Counts
    long long numDimensions;
    long long numElements;
    long long numSelected;
    long long numVisible;

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
    DataSetObject *parent;

    QBitArray visibility;
    QVector<int> selectionGroup;

    QVector<qreal> dimSums;
    QVector<qreal> minimumValues;
    QVector<qreal> maximumValues;
    QVector<qreal> meanValues;
    QVector<qreal> standardDeviations;
    QVector<qreal> covarianceMatrix;
    QVector<qreal> correlationMatrix;
};

class DataSetObject
{
    friend class DataObject;

public:
    DataSetObject();

    DataObject* at(int i) { return dataObjects[i]; }
    bool isEmpty() { return dataObjects.isEmpty(); }
    int size() { return dataObjects.size(); }
    hardwareTopology* hwTopo() { return hw; }
    void setConsole(console *icon) { con = icon; }

    // Set data and hardware
    int addData(QString filename);
    int setHardwareTopology(QString filename);

    // Triggers
    bool selectionDefined();
    void selectionChanged();
    void visibilityChanged();

    // Selection/Visibility
    void hideUnselected();
    void showAll();
    void deselectAll();
    void hideSelected();
    void setSelectionMode(selection_mode mode);
    void selectAll();
    void selectAllVisible();

    // Selection Queries
    void selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes);
    void selectByMultiDimRange(QVector<QString> dims, QVector<qreal> mins, QVector<qreal> maxes);
    void selectByVarName(QString str);
    void selectByResource(hardwareResourceNode *node);

    // Calculated values
    int numSelected();
    int numUnselected();
    int numTotal();

    // Clustering
    void worstPair(QVector<QVector<qreal> > *dm, int *i0, int *i1);
    int clusterHardware();

private:
    console *con;
    hardwareTopology *hw;
    QVector<DataObject*> dataObjects;

    int selGroup;
    int selMode;
};

#endif // DATAOBJECT_H
