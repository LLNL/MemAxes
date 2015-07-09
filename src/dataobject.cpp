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

#include "dataobject.h"
#include "parseUtil.h"

#include <iostream>
#include <algorithm>
#include <functional>

#include <QFile>
#include <QTextStream>

DataObject::DataObject()
{
    numDimensions = 0;
    numElements = 0;
    numSelected = 0;
    numVisible = 0;

    topo = NULL;

    selMode = MODE_NEW;
}

int DataObject::loadHardwareTopology(QString filename)
{
    topo = new HWTopo();
    int err = topo->loadHardwareTopologyFromXML(filename);
    return err;
}

int DataObject::loadData(QString filename)
{
    int err = parseCSVFile(filename);
    if(err)
        return err;

    calcStatistics();
    constructSortedLists();

    return 0;
}

void DataObject::allocate()
{
    numDimensions = meta.size();
    numElements = vals.size() / numDimensions;

    begin = vals.begin();
    end = vals.end();

    clusterTrees.resize(numDimensions,NULL);

    visibility.resize(numElements);
    visibility.fill(VISIBLE);

    selectionGroup.resize(numElements);
    selectionGroup.fill(0); // all belong to 0 (unselected)

    selectionSets.push_back(ElemSet());
    selectionSets.push_back(ElemSet());
}

int DataObject::selected(ElemIndex index)
{
    return selectionGroup.at((int)index);
}

bool DataObject::visible(ElemIndex index)
{
    return visibility.at((int)index);
}

bool DataObject::selectionDefined()
{
    return numSelected > 0;
}

void DataObject::selectData(ElemIndex index, int group)
{
    if(visible(index))
    {
        selectionGroup[index] = group;
        selectionSets.at(group).insert(index);
        numSelected++;
    }
}

void DataObject::selectAll(int group)
{
    selectionGroup.fill(group);

    for(ElemIndex i=0; i<numElements; i++)
    {
        selectionSets.at(group).insert(i);
    }

    numSelected = numElements;
}

void DataObject::deselectAll()
{
    selectionGroup.fill(0);

    for(unsigned int i=0; i<selectionSets.size(); i++)
        selectionSets.at(i).clear();

    numSelected = 0;
}

void DataObject::selectAllVisible(int group)
{
    ElemIndex elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(visible(elem))
            selectData(elem,group);
    }
}

void DataObject::showData(unsigned int index)
{
    if(!visible(index))
    {
        visibility[index] = VISIBLE;
        numVisible++;
    }
}

void DataObject::hideData(unsigned int index)
{
    if(visible(index))
    {
        visibility[index] = INVISIBLE;
        numVisible--;
    }
}

void DataObject::showAll()
{
    visibility.fill(VISIBLE);
    numVisible = numElements;
}

void DataObject::hideAll()
{
    visibility.fill(INVISIBLE);
    numVisible = 0;
}

// TODO: use a range query instead of checking everything
ElemSet& DataObject::createSourceFileQuery(QString str)
{
    ElemSet& selSet = *(new ElemSet);

    ElemIndex elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        if(fileNames[elem] == str)
            selSet.insert(elem);
    }
    return selSet;
}

struct indexedValueLtFunctor
{
    indexedValueLtFunctor(const struct indexedValue _it) : it(_it) {}

    struct indexedValue it;

    bool operator()(const struct indexedValue &other)
    {
        return it.val < other.val;
    }
};

ElemSet& DataObject::createDimRangeQuery(int dim, qreal vmin, qreal vmax)
{
    con->log("DIMRANGE"+QString::number(dim)
             +"="+ QString::number(vmin)
             +":"+QString::number(vmax));

    ElemSet& selSet = *(new ElemSet);

    std::vector<indexedValue>::iterator itMin;
    std::vector<indexedValue>::iterator itMax;

    struct indexedValue ivMinQuery;
    ivMinQuery.val = vmin;

    struct indexedValue ivMaxQuery;
    ivMaxQuery.val = vmax;

    if(ivMinQuery.val <= this->minimumValues[dim])
    {
        itMin = dimSortedLists.at(dim).begin();
    }
    else
    {
        itMin = std::find_if(dimSortedLists.at(dim).begin(),
                             dimSortedLists.at(dim).end(),
                             indexedValueLtFunctor(ivMinQuery));
    }

    if(ivMaxQuery.val >= this->maximumValues[dim])
    {
        itMax = dimSortedLists.at(dim).end();
    }
    else
    {
        itMax = std::find_if(dimSortedLists.at(dim).begin(),
                             dimSortedLists.at(dim).end(),
                             indexedValueLtFunctor(ivMaxQuery));
    }

    for(/*itMin*/; itMin != itMax; itMin++)
    {
        selSet.insert(itMin->idx);
    }

    return selSet;
}

ElemSet& DataObject::createMultiDimRangeQuery(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes)
{
    ElemSet& selSet = *(new ElemSet);

    for(int d=0; d<dims.size(); d++)
    {
        ElemSet tmp = createDimRangeQuery(dims[d],mins[d],maxes[d]);
        ElemSet tmpunion;

        std::set_union(selSet.begin(),selSet.end(),tmp.begin(),tmp.end(),std::inserter(tmpunion,tmpunion.begin()));

        std::copy(tmpunion.begin(),tmpunion.end(),std::inserter(selSet,selSet.begin()));
    }

    return selSet;
}

ElemSet& DataObject::createVarNameQuery(QString str)
{
    ElemSet& selSet = *(new ElemSet);

    ElemIndex elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        if(varNames[elem] == str)
            selSet.insert(elem);
    }

    return selSet;
}

ElemSet& DataObject::createResourceQuery(HWNode *node)
{
    return node->allSamples;
}

void DataObject::hideSelected()
{
    ElemIndex elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(selected(elem))
            hideData(elem);
    }
}

void DataObject::hideUnselected()
{
    ElemIndex elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(!selected(elem) && visible(elem))
            hideData(elem);
    }
}

void DataObject::selectSet(ElemSet &query, int group)
{
    ElemSet *newSel = NULL;
    if(selMode == MODE_NEW)
    {
        newSel = &query;
    }
    else if(selMode == MODE_APPEND)
    {
        newSel = new ElemSet();
        std::set_union(selectionSets.at(group).begin(),
                       selectionSets.at(group).end(),
                       query.begin(),
                       query.end(),
                       std::inserter(*newSel,
                                     newSel->begin()));
    }
    else if(selMode == MODE_FILTER)
    {
        newSel = new ElemSet();
        std::set_intersection(selectionSets.at(group).begin(),
                              selectionSets.at(group).end(),
                              query.begin(),
                              query.end(),
                              std::inserter(*newSel,
                                            newSel->begin()));
    }
    else
    {
        std::cerr << "MANATEES! EVERYWHERE!" << std::endl;
        return;
    }

    // Reprocess groups
    selectionSets.at(group).clear();
    selectionGroup.fill(0);

    for(ElemSet::iterator it = newSel->begin();
        it != newSel->end();
        it++)
    {
        selectData(*it,group);
    }
}

void DataObject::collectTopoSamples()
{
    topo->collectSamples(this,&allElems);
}

int DataObject::getDimensions()
{
    sourceDim = this->meta.indexOf("source");
    if(sourceDim < 0)
    {
        std::cerr << "Error: source dimension not found!" << std::endl;
        return -1;
    }

    lineDim = this->meta.indexOf("line");
    if(lineDim < 0)
    {
        std::cerr << "Error: line dimension not found!" << std::endl;
        return -1;
    }

    variableDim = this->meta.indexOf("variable");
    if(variableDim < 0)
    {
        variableDim = this->meta.indexOf("instruction");
    }
    if(variableDim < 0)
    {
        std::cerr << "Error: variable dimension not found!" << std::endl;
        return -1;
    }

    dataSourceDim = this->meta.indexOf("data_src");
    if(dataSourceDim < 0)
    {
        dataSourceDim = this->meta.indexOf("data_source");
    }
    if(dataSourceDim < 0)
    {
        dataSourceDim = this->meta.indexOf("dataSource");
    }
    if(dataSourceDim < 0)
    {
        std::cerr << "Error: data source dimension not found!" << std::endl;
        return -1;
    }

    latencyDim = this->meta.indexOf("latency");
    if(latencyDim < 0)
    {
        std::cerr << "Error: latency dimension not found!" << std::endl;
        return -1;
    }
    cpuDim = this->meta.indexOf("cpu");
    if(cpuDim < 0)
    {
        std::cerr << "Error: cpu dimension not found!" << std::endl;
        return -1;
    }

    timeDim = this->meta.indexOf("time");
    if(timeDim < 0)
    {
        std::cerr << "Error: time dimension not found!" << std::endl;
        return -1;
    }

    return 0;
}

int DataObject::parseCaliFile(QString caliFileName)
{
    return 0;
}

int DataObject::parseCSVFile(QString dataFileName)
{
    // Open the file
    QFile dataFile(dataFileName);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;

    // Get number of lines
    QTextStream dataStream(&dataFile);
    unsigned long long numLines = 0;
    while(!dataStream.atEnd())
    {
        dataStream.readLine();
        numLines++;
    }
    numLines--; // one header line

    // rewind
    dataStream.device()->seek(0);

    QString line;
    QStringList lineValues;

    // Get metadata from first line
    line = dataStream.readLine();
    this->meta = line.split(',');
    this->numDimensions = this->meta.size();

    // Get dimensions from metadata
    this->getDimensions();

    QVector<QString> varVec;
    QVector<QString> sourceVec;

    unsigned long long minTime = std::numeric_limits<unsigned long long>::max();
    unsigned long long maxTime = 0;

    // Get data
    progress = 0;
    ElemIndex elem = 0;
    while(!dataStream.atEnd())
    {
        line = dataStream.readLine();
        lineValues = line.split(',');

        if(lineValues.size() != this->numDimensions)
        {
            std::cerr << "ERROR: element dimensions do not match metadata!" << std::endl;
            std::cerr << "At element " << elem << std::endl;
            return -1;
        }

        // Process individual dimensions differently
        for(int i=0; i<lineValues.size(); i++)
        {
            QString tok = lineValues[i];
            if(i==variableDim)
            {
                ElemIndex uid = createUniqueID(varVec,tok);
                this->vals.push_back(uid);
                varNames.push_back(tok);
            }
            else if(i==sourceDim)
            {
                ElemIndex uid = createUniqueID(sourceVec,tok);
                this->vals.push_back(uid);
                fileNames.push_back(tok);
            }
            else if(i==dataSourceDim)
            {
                int depth = dseToDepth(tok.toULongLong());
                this->vals.push_back(depth);
            }
            else if(i==timeDim)
            {
                unsigned long long time = tok.toULongLong();
                minTime = std::min(time,minTime);
                maxTime = std::max(time,maxTime);

                this->vals.push_back(time);

            }
            else
            {
                this->vals.push_back(tok.toLongLong());
            }
        }

        allElems.insert(elem);
        elem++;

        if(numLines/elem > progress)
        {
            progress = numLines / elem;
        }
    }

    // Close and return
    dataFile.close();

    for(int i=timeDim; i<(int)vals.size(); i+=numDimensions)
    {
        // scale time to [0,1M]
        vals[i] = scale(vals[i],minTime,maxTime,0,1000000);
    }

    this->allocate();

    return 0;
}

void DataObject::setSelectionMode(selection_mode mode, bool silent)
{
    selMode = mode;

    if(silent)
        return;

    QString selcmd("select MODE=");
    switch(mode)
    {
        case(MODE_NEW):
            selcmd += "new";
            break;
        case(MODE_APPEND):
            selcmd += "append";
            break;
        case(MODE_FILTER):
            selcmd += "filter";
            break;
    }
    con->log(selcmd);
}

void DataObject::calcStatistics()
{
    dimSums.resize(this->numDimensions);
    minimumValues.resize(this->numDimensions);
    maximumValues.resize(this->numDimensions);
    meanValues.resize(this->numDimensions);
    standardDeviations.resize(this->numDimensions);

    covarianceMatrix.resize(this->numDimensions*this->numDimensions);
    correlationMatrix.resize(this->numDimensions*this->numDimensions);

    qreal firstVal = *(this->begin);
    dimSums.fill(0);
    minimumValues.fill(9999999999);
    maximumValues.fill(0);
    meanValues.fill(0);
    standardDeviations.fill(0);

    // Means and combined means
    QVector<qreal>::Iterator p;
    ElemIndex elem;
    qreal x, y;

    QVector<qreal> meanXY;
    meanXY.resize(this->numDimensions*this->numDimensions);
    meanXY.fill(0);
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        for(int i=0; i<this->numDimensions; i++)
        {
            x = *(p+i);
            dimSums[i] += x;
            minimumValues[i] = std::min(x,minimumValues[i]);
            maximumValues[i] = std::max(x,maximumValues[i]);

            for(int j=0; j<this->numDimensions; j++)
            {
                y = *(p+j);
                meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] += x*y;
            }
        }
    }

    // Divide by this->numElements to get mean
    for(int i=0; i<this->numDimensions; i++)
    {
        meanValues[i] = dimSums[i] / (qreal)this->numElements;
        for(int j=0; j<this->numDimensions; j++)
        {
            meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] /= (qreal)this->numElements;
        }
    }

    // Covariance = E(XY) - E(X)*E(Y)
    for(int i=0; i<this->numDimensions; i++)
    {
        for(int j=0; j<this->numDimensions; j++)
        {
            covarianceMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] =
                meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] - meanValues[i]*meanValues[j];
        }
    }

    // Standard deviation of each dim
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        for(int i=0; i<this->numDimensions; i++)
        {
            x = *(p+i);
            standardDeviations[i] += (x-meanValues[i])*(x-meanValues[i]);
        }
    }

    for(int i=0; i<this->numDimensions; i++)
    {
        standardDeviations[i] = sqrt(standardDeviations[i]/(qreal)this->numElements);
    }

    // Correlation Coeff = cov(xy) / stdev(x)*stdev(y)
    for(int i=0; i<this->numDimensions; i++)
    {
        for(int j=0; j<this->numDimensions; j++)
        {
            correlationMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] =
                    covarianceMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] /
                    (standardDeviations[i]*standardDeviations[j]);
        }
    }
}

void DataObject::constructSortedLists()
{
    dimSortedLists.resize(this->numDimensions);
    for(int d=0; d<this->numDimensions; d++)
    {
        for(int e=0; e<this->numElements; e++)
        {
            struct indexedValue di;
            di.idx = e;
            di.val = at(e,d);
            dimSortedLists.at(d).push_back(di);
        }
        std::sort(dimSortedLists.at(d).begin(),dimSortedLists.at(d).end());
    }
}

void DataObject::createClusterTree(int dim, METRIC_TYPE m)
{
    clusterTrees[dim] = new DataClusterTree();
    clusterTrees[dim]->build(this,dim,m);
}

