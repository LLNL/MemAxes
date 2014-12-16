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
using namespace std;

#include <QFile>
#include <QTextStream>

DataObject::DataObject()
{
    numSelected = 0;
    topo = NULL;
}

void DataObject::allocate()
{
    numDimensions = meta.size();
    numElements = vals.size() / numDimensions;

    begin = vals.begin();
    end = vals.end();

    visibility.resize(numElements);
    visibility.fill(VISIBLE);

    selectionGroup.resize(numElements);
    selectionGroup.fill(0); // all belong to 0 (unselected)
}

int DataObject::selected(unsigned int index)
{
    return selectionGroup[index];
}

bool DataObject::visible(unsigned int index)
{
    return visibility[index];
}

bool DataObject::selectionDefined()
{
    return numSelected > 0;
}

bool DataObject::skip(unsigned int index)
{
    return visible(index) && (!selectionDefined() || selected(index));
}

void DataObject::selectData(unsigned int index, int group)
{
    if(group == -1)
        group = parent->selGroup;

    if(visible(index))
    {
        selectionGroup[index] = group;
        numSelected++;
    }
}

void DataObject::deselectData(unsigned int index)
{
    if(visible(index))
    {
        selectionGroup[index] = 0;
        numSelected--;
    }
}

void DataObject::logicalSelectData(unsigned int index, bool select, int group)
{
    if(group == -1)
        group = parent->selGroup;

    if(select && (parent->selMode == MODE_APPEND || parent->selMode == MODE_NEW))
        this->selectData(index,group);
    else if(!select && parent->selMode == MODE_FILTER)
        this->deselectData(index);
}

void DataObject::selectAll()
{
    selectionGroup.fill(parent->selGroup);
    numSelected = numElements;
}

void DataObject::deselectAll()
{
    selectionGroup.fill(0);
    numSelected = 0;
}

void DataObject::selectAllVisible()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(visible(elem))
            selectData(elem);
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

void DataObject::selectBySourceFileName(QString str)
{
    if(parent->selMode == MODE_NEW)
        deselectAll();

    bool select;
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        select = (fileNames[elem] == str);
        logicalSelectData(elem,select);
    }
}

void DataObject::selectByDimRange(int dim, qreal vmin, qreal vmax)
{
    if(parent->selMode == MODE_NEW)
        deselectAll();

    bool select;
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        select = within(*(p+dim),vmin,vmax);
        logicalSelectData(elem,select);
    }
}

void DataObject::selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes)
{
    if(parent->selMode == MODE_NEW)
        deselectAll();

    bool select;
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        select = true;

        for(int i=0; i<dims.size(); i++)
        {
            if(!within(*(p+dims[i]),mins[i],maxes[i]))
            {
                select = false;
                break;
            }
        }

        logicalSelectData(elem,select);
    }
}

void DataObject::selectByVarName(QString str)
{
    if(parent->selMode == MODE_NEW)
        deselectAll();

    bool select;
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        select = (varNames[elem] == str);
        logicalSelectData(elem,select);
    }
}

void DataObject::selectByResource(hardwareResourceNode *node)
{
    if(parent->selMode == MODE_NEW)
        deselectAll();

    QVector<int> selelems;
    QVector<int> *samples = &node->sampleSets[this].totSamples;
    for(int i=0; i<samples->size(); i++)
    {
        int elem = samples->at(i);
        if(parent->selMode == MODE_NEW || parent->selMode == MODE_APPEND)
            this->selectData(elem);
        else if(parent->selMode == MODE_FILTER && selected(elem))
            selelems.push_back(elem);
    }

    if(parent->selMode == MODE_FILTER)
    {
        deselectAll();
        for(int i=0; i<selelems.size(); i++)
        {
            this->selectData(selelems[i]);
        }
    }
}

void DataObject::hideSelected()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(selected(elem))
            hideData(elem);
    }
}

void DataObject::hideUnselected()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(!selected(elem) && visible(elem))
            hideData(elem);
    }
}

void DataObject::collectTopoSamples(hardwareTopology *hw, bool sel)
{
    topo = hw;

    // Reset info
    for(int i=0; i<topo->allHardwareResourceNodes.size(); i++)
    {
        topo->allHardwareResourceNodes[i]->sampleSets[this].totCycles = 0;
        topo->allHardwareResourceNodes[i]->sampleSets[this].selCycles = 0;
        topo->allHardwareResourceNodes[i]->sampleSets[this].totSamples.clear();
        topo->allHardwareResourceNodes[i]->sampleSets[this].selSamples.clear();
        topo->allHardwareResourceNodes[i]->transactions = 0;
    }

    // Go through each sample and add it to the right topo node
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        // Get vars
        int dse = *(p+dataSourceDim);
        int cpu = *(p+cpuDim);
        int cycles = *(p+latencyDim);

        // Search for nodes
        hardwareResourceNode *cpuNode = topo->CPUIDMap[cpu];
        hardwareResourceNode *node = cpuNode;

        // Update data for serving resource
        node->sampleSets[this].totSamples.push_back(elem);
        node->sampleSets[this].totCycles += cycles;

        if(!(sel && !selected(elem)))
        {
            node->sampleSets[this].selSamples.push_back(elem);
            node->sampleSets[this].selCycles += cycles;
        }

        if(dse == -1)
            continue;

        // Go up to data source
        for( /*init*/; dse>0 && node->parent; dse--, node=node->parent)
        {
            if(!(sel && !selected(elem)))
            {
                node->transactions++;
            }
        }

        // Update data for core
        node->sampleSets[this].totSamples.push_back(elem);
        node->sampleSets[this].totCycles += cycles;

        if(!(sel && !selected(elem)))
        {
            node->sampleSets[this].selSamples.push_back(elem);
            node->sampleSets[this].selCycles += cycles;
        }
    }
}

int DataObject::parseCSVFile(QString dataFileName)
{
    // Open the file
    QFile dataFile(dataFileName);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;

    // Create text stream
    QTextStream dataStream(&dataFile);
    QString line;
    QStringList lineValues;
    qint64 elemid = 0;

    // Get metadata from first line
    line = dataStream.readLine();
    this->meta = line.split(',');
    this->numDimensions = this->meta.size();

    sourceDim = this->meta.indexOf("source");
    lineDim = this->meta.indexOf("line");
    variableDim = this->meta.indexOf("variable");
    dataSourceDim = this->meta.indexOf("dataSource");
    indexDim = this->meta.indexOf("index");
    latencyDim = this->meta.indexOf("latency");
    nodeDim = this->meta.indexOf("node");
    cpuDim = this->meta.indexOf("cpu");
    xDim = this->meta.indexOf("xidx");
    yDim = this->meta.indexOf("yidx");
    zDim = this->meta.indexOf("zidx");

    // Get data
    while(!dataStream.atEnd())
    {
        line = dataStream.readLine();
        lineValues = line.split(',');

        if(lineValues.size() != this->numDimensions)
        {
            cerr << "ERROR: element dimensions do not match metadata!" << endl;
            cerr << "At element " << elemid << endl;
            return -1;
        }

        // Process individual dimensions differently
        for(int i=0; i<lineValues.size(); i++)
        {
            if(i==variableDim)
            {
                this->vals.push_back(-1);
                varNames.push_back(lineValues[i]);
            }
            else if(i==sourceDim)
            {
                this->vals.push_back(-1);
                fileNames.push_back(lineValues[i]);
            }
            else if(i==dataSourceDim)
            {
                int dseVal = lineValues[i].toInt(NULL,16);
                this->vals.push_back(dseDepth(dseVal));
            }
            else
            {
                this->vals.push_back(lineValues[i].toLongLong());
            }
        }

        elemid++;
    }

    // Close and return
    dataFile.close();

    this->allocate();

    return 0;
}

void DataObject::calcStatistics(int group)
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
    minimumValues.fill(firstVal);
    maximumValues.fill(firstVal);
    meanValues.fill(0);
    standardDeviations.fill(0);

    // Means and combined means
    QVector<qreal>::Iterator p;
    long long elem;
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
            minimumValues[i] = min(x,minimumValues[i]);
            maximumValues[i] = max(x,maximumValues[i]);

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

qreal DataObject::distanceHardware(DataObject *dso)
{
    // dso must have same hardware topology
    if(topo != dso->topo)
        return -1;
    if(this == dso)
        return 0;

    // Get difference of cycles for each level
    QVector<qreal> depthDistances;
    int depth = topo->hardwareResourceMatrix.size();
    for(int d=0; d<depth; d++)
    {
        qreal ddist = 0;
        int width = topo->hardwareResourceMatrix[d].size();
        for(int w=0; w<width; w++)
        {
            hardwareResourceNode *node = topo->hardwareResourceMatrix[d][w];
            struct SampleSet ss1 = node->sampleSets[this];
            struct SampleSet ss2 = node->sampleSets[dso];
            ddist += abs(ss1.totCycles - ss2.totCycles);
        }
        depthDistances.push_back(ddist);
    }

    // Total distance is some weighted sum of depth distances
    qreal dist = 0;
    for(int d=0; d<depth; d++)
    {
        dist += depthDistances[d];
    }

    return dist;
}

/*
 * DataSetObject
 */

DataSetObject::DataSetObject()
{
    hw = NULL;
    selMode = MODE_NEW;
    selGroup = 1;
}

int DataSetObject::addData(QString filename)
{
    DataObject *dobj = new DataObject();
    int ret = dobj->parseCSVFile(filename);
    if(ret != 0)
    {
        con->log("Error Loading Dataset : "+filename);
        return ret;
    }

    dobj->calcStatistics();
    dobj->parent = this;
    dataObjects.push_back(dobj);

    con->log("Added Dataset : "+filename);
    return 0;
}

int DataSetObject::setHardwareTopology(QString filename)
{
    hw = new hardwareTopology();
    int ret = hw->loadHardwareTopologyFromXML(filename);
    if(ret != 0)
    {
        con->log("Error Loading Hardware Topology : "+filename);
        return ret;
    }
    con->log("Loaded Hardware Topology : "+filename);
    std::cout << "loaded hardware topology :" << filename.toStdString() << std::endl;
    return 0;
}

#define FOR_EACH_DATA(func) \
    for(int d=0; d<dataObjects.size(); d++) \
        dataObjects[d]->func;

void DataSetObject::hideUnselected()
{
    QString selcmd("hide UNSELECTED");
    con->log(selcmd);

    FOR_EACH_DATA(hideUnselected());
}

void DataSetObject::showAll()
{
    QString selcmd("show ALL");
    con->log(selcmd);

    FOR_EACH_DATA(showAll());
}

void DataSetObject::deselectAll()
{
    QString selcmd("deselect ALL");
    con->log(selcmd);

    FOR_EACH_DATA(deselectAll());
}

void DataSetObject::hideSelected()
{
    QString selcmd("hide SELECTED");
    con->log(selcmd);

    FOR_EACH_DATA(hideSelected());
}

void DataSetObject::setSelectionMode(selection_mode mode)
{
    QString selcmd("select MODE=");
    switch(mode)
    {
        case(MODE_NEW):
            selcmd += "filter";
            break;
        case(MODE_APPEND):
            selcmd += "append";
            break;
        case(MODE_FILTER):
            selcmd += "filter";
            break;
    }
    con->log(selcmd);

    selMode = mode;
}

void DataSetObject::selectAll()
{
    QString selcmd("select ALL");
    con->log(selcmd);

    FOR_EACH_DATA(selectAll());
}

void DataSetObject::selectAllVisible()
{
    QString selcmd("select VISIBLE");
    con->log(selcmd);

    FOR_EACH_DATA(selectAllVisible());
}

bool DataSetObject::selectionDefined()
{
    for(int d=0; d<dataObjects.size(); d++)
        if(dataObjects[d]->selectionDefined())
            return true;
    return false;
}

void DataSetObject::selectionChanged()
{
    bool sel = selectionDefined();
    FOR_EACH_DATA(collectTopoSamples(hw,sel));
}

void DataSetObject::visibilityChanged()
{
    bool sel = selectionDefined();
    FOR_EACH_DATA(collectTopoSamples(hw,sel));
}

void DataSetObject::selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes)
{
    QString selcmd("select DIMRANGE ");
    for(int i=0; i<dims.size(); i++)
    {
        selcmd += QString::number(dims[i]) + "=" ;
        selcmd += QString::number(mins[i]) + ":" ;
        selcmd += QString::number(maxes[i]) + " ";
    }
    con->log(selcmd);

    FOR_EACH_DATA(selectByMultiDimRange(dims,mins,maxes));
}

void DataSetObject::selectByMultiDimRange(QVector<QString> dims, QVector<qreal> mins, QVector<qreal> maxes)
{
    QString selcmd("select DIMRANGE ");
    for(int i=0; i<dims.size(); i++)
    {
        selcmd += dims[i] + "=" ;
        selcmd += QString::number(mins[i]) + ":" ;
        selcmd += QString::number(maxes[i]) + " ";
    }
    con->log(selcmd);

    for(int d=0; d<dataObjects.size(); d++)
    {
        QVector<int> intDims;
        for(int i=0; i<dims.size(); i++)
            intDims.push_back(dataObjects[d]->meta.indexOf(dims[i]));

        dataObjects[d]->selectByMultiDimRange(intDims,mins,maxes);
    }
}

void DataSetObject::selectByVarName(QString str)
{
    QString selcmd("select VARIABLE "+str);
    con->log(selcmd);

    FOR_EACH_DATA(selectByVarName(str));
}

void DataSetObject::selectByResource(hardwareResourceNode *node)
{
    QString selcmd("select RESOURCE "+node->name+"="+QString::number(node->id));
    con->log(selcmd);

    FOR_EACH_DATA(selectByResource(node));
}

int DataSetObject::numSelected()
{
    int ns = 0;
    for(int d=0; d<dataObjects.size(); d++)
        ns += dataObjects[d]->numSelected;
    return ns;
}

int DataSetObject::numUnselected()
{
    int nus = 0;
    for(int d=0; d<dataObjects.size(); d++)
        nus += dataObjects[d]->numElements - dataObjects[d]->numSelected;
    return nus;
}

int DataSetObject::numTotal()
{
    int ne = 0;
    for(int d=0; d<dataObjects.size(); d++)
        ne += dataObjects[d]->numElements;
    return ne;
}

void DataSetObject::worstPair(QVector<QVector<qreal> > *dm, int *i0, int *i1)
{
    // Find most distant pair and return it
    qreal maxdist = 0;
    int maxd0, maxd1;
    for(int d0=0; d0<dm->size(); d0++)
    {
        for(int d1=d0; d0<dm->at(d0).size(); d0++)
        {
            if(dm->at(d0).at(d1) > maxdist)
            {
                maxd0 = d0;
                maxd1 = d1;
                maxdist = dm->at(d0).at(d1);
            }
        }
    }

    *i0 = maxd0;
    *i1 = maxd1;
}

int factorial(int x, int result = 1) {
    if(x == 1) return result; else return factorial(x - 1, x * result);
}

#define PAIRS(n,r) factorial(n) / (factorial(n-r)*factorial(r))

int DataSetObject::clusterHardware()
{
    int clusters = 0;

    int n = dataObjects.size();

    // Create distance matrix
    QVector<QVector<qreal> > dm;
    dm.resize(n);
    for(int d0=0; d0<n; d0++)
    {
        dm[d0].resize(d0);
        dm[d0].fill(0);
        for(int d1=0; d1<d0; d1++)
        {
            dm[d0][d1] = dataObjects[d0]->distanceHardware(dataObjects[d1]);
        }
    }

    // Create adjacency matrix (fully connected)
    QVector<QVector<int> > adj;
    adj.resize(n);
    for(int d0=0; d0<n; d0++)
    {
        for(int d1=0; d1<n; d1++)
        {
            if(d0 != d1)
                adj[d0].push_back(d1);
        }
    }

    // Until we have everything paired up, remove worst pair
    int pairCount = 0;
    int fullPairs = n/2;
    while(pairCount < fullPairs)
    {
        int i,j;
        worstPair(&dm,&i,&j);

        int rm0 = adj[i].indexOf(j);
        int rm1 = adj[j].indexOf(i);

        adj[i].remove(rm0);
        adj[j].remove(rm1);

        if(adj[i].size() == 1)
        {
            dm.remove(adj[i][0]);
            dm.remove(i);
            pairCount--;
        }

        if(adj[j].size() == 1)
        {
            dm.remove(adj[j][0]);
            dm.remove(j);
            pairCount--;
        }
    }

    
    return clusters;
}
