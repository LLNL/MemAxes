#include "dataobject.h"

#include <iostream>
using namespace std;

#include <QFile>
#include <QTextStream>

DataObject::DataObject()
{
    numSelected = 0;
    topo = NULL;
    selectionMode = MODE_NEW;
}

void DataObject::init()
{
    numDimensions = meta.size();
    numElements = vals.size() / numDimensions;

    begin = vals.begin();
    end = vals.end();

    selection.resize(numElements);
    selection.fill(VISIBLE);
}

void DataObject::setHardwareTopology(hardwareTopology *hw, bool sel)
{
    topo = hw;

    for(int i=0; i<topo->allHardwareResourceNodes.size(); i++)
    {
        SampleIdxVector *samples = &topo->allHardwareResourceNodes[i]->sampleSets[this].first;
        int *numCycles = &topo->allHardwareResourceNodes[i]->sampleSets[this].second;

        samples->clear();
        *numCycles = 0;
    }

    // Go through each sample and add it to the right topo node
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        if(sel && !selected(elem))
            continue;

        int dse = *(p+dataSourceDim);
        int cpu = *(p+cpuDim);
        //int numa = *(p+nodeDim);
        int cycles = *(p+latencyDim);

        hardwareResourceNode *cpuNode = topo->CPUIDMap[cpu];
        //hardwareResourceNode *numaNode = topo->NUMAIDMap[numa];

        hardwareResourceNode *node = cpuNode;

        SampleIdxVector *samples = &node->sampleSets[this].first;
        int *numCycles = &node->sampleSets[this].second;

        samples->push_back(elem);
        *numCycles += cycles;

        if(dse == -1)
            continue;

        // Go up to data source
        for( ; dse>0; dse--)
            node = node->parent;

        samples = &node->sampleSets[this].first;
        numCycles = &node->sampleSets[this].second;

        samples->push_back(elem);
        *numCycles += cycles;
    }
}

void DataObject::selectData(unsigned int index)
{
    if(selection[index] == VISIBLE)
    {
        selection[index] = SELECTED;
        numSelected++;
    }
}

void DataObject::deselectData(unsigned int index)
{
    if(selection[index] == SELECTED)
    {
        selection[index] = VISIBLE;
        numSelected--;
    }
}

void DataObject::logicalSelectData(unsigned int index, bool select)
{
    if(select && (selectionMode == MODE_APPEND || selectionMode == MODE_NEW))
        this->selectData(index);
    else if(!select && selectionMode == MODE_FILTER)
        this->deselectData(index);
}

void DataObject::selectAll()
{
    showAll();
    selectAllVisible();
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

void DataObject::deselectAll()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
        deselectData(elem);
}

void DataObject::showData(unsigned int index)
{
    if(!visible(index))
    {
        selection[index] = VISIBLE;
        numVisible++;
    }
}

void DataObject::showAll()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(!visible(elem))
            showData(elem);
    }
}

void DataObject::hideData(unsigned int index)
{
    if(visible(index))
    {
        selection[index] = INVISIBLE;
        numVisible--;
    }
}

void DataObject::hideAll()
{
    selection.fill(INVISIBLE);
    numVisible = 0;
}

void DataObject::selectBySourceFileName(QString str)
{
    if(selectionMode == MODE_NEW)
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
    if(selectionMode == MODE_NEW)
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
    if(selectionMode == MODE_NEW)
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
    if(selectionMode == MODE_NEW)
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

bool DataObject::selected(unsigned int index)
{
    return selection[index] == SELECTED;
}

bool DataObject::visible(unsigned int index)
{
    return selection[index] != INVISIBLE;
}

bool DataObject::selectionDefined()
{
    return numSelected > 0;
}

bool DataObject::skip(unsigned int index)
{
    bool sel = !selectionDefined() || selected(index);
    return sel && visible(index);
}

// Things get messy here

int createUniqueID(QVector<QString> existing, QString name)
{
    for(int i=0; i<existing.size(); i++)
    {
        if(existing[i] == name)
            return i;
    }
    existing.push_back(name);
    return existing.size()-1;
}

int encDepth(int enc)
{
    int src = enc & 0xF;
    switch(src)
    {
        case(0x0): return -1; // at least L3
        case(0x1): return 1; // L1
        case(0x2): return 1; // cache hit pending (don't draw)
        case(0x3): return 2; // L2
        case(0x4): return 3; // L3
        case(0x5): return 3; // from another core L2/L1 (clean)
        case(0x6): return 3; // from another core L2/L1 (dirty)
        case(0x7): return -1; // no LLC now
        case(0x8): return 4; // local ram?
        case(0x9): return -1; // reserved (shouldn't happen)
        case(0xA): return 4; // local RAM (clean)
        case(0xB): return 4; // remote RAM (clean)
        case(0xC): return 4; // local RAM (dirty)
        case(0xD): return 4; // remote RAM (dirty)
        case(0xE): return -1; // I/O
        case(0xF): return -1; // Uncacheable
    }

    return -1;
}

int encDirty(int enc)
{
    int src = enc & 0xF;
    switch(src)
    {
        case(0x5): return 0; // from another core L2/L1 (clean)
        case(0x6): return 1; // from another core L2/L1 (dirty)
        case(0xA): return 0; // local RAM (clean)
        case(0xB): return 0; // remote RAM (clean)
        case(0xC): return 1; // local RAM (dirty)
        case(0xD): return 1; // remote RAM (dirty)
    }

    // N/A
    return -1;
}


std::string encToString(int enc)
{
    int src = enc & 0xF;

    switch(src)
    {
        case(0x0): return "Unknown L3 Miss";
        case(0x1): return "L1";
        case(0x2): return "TLB";
        case(0x3): return "L2";
        case(0x4): return "L3";
        case(0x5): return "L3 Snoop (clean)"; // from another core L2/L1
        case(0x6): return "L3 Snoop (dirty)"; // from another core L2/L1
        case(0x7): return "LLC Snoop (dirty)";
        case(0x8): return "L3 Miss (dirty)"; // local ram?
        case(0x9): return "MONKEYS"; // reserved (shouldn't happen)
        case(0xA): return "Local RAM";
        case(0xB): return "Remote RAM";
        case(0xC): return "Local RAM";
        case(0xD): return "Remote RAM";
        case(0xE): return "I/O";
        case(0xF): return "Uncacheable";
    }

    return "???"; // really weird
}


int encStlb(int enc)
{
    return enc & 0x10;
}

int encLocked(int enc)
{
    return enc & 0x20;
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

    // HARD CODE

    // Get metadata from first line
    line = dataStream.readLine();
    this->meta = line.split(',');
    this->meta.insert(7,"STLB Miss");
    this->meta.insert(7,"Locked");
    this->meta.insert(7,"Dirtiness");
    this->numDimensions = this->meta.size();

    // Get data
    while(!dataStream.atEnd())
    {
        line = dataStream.readLine();
        lineValues = line.split(',');

        if(lineValues.size() != this->numDimensions-3)
        {
            cerr << "ERROR: element dimensions do not match metadata!" << endl;
            cerr << "At element " << elemid << endl;
            return -1;
        }

        // hack for different kinds of values
        for(int i=0; i<lineValues.size(); i++)
        {
            if(i==0) // data object
            {
                this->vals.push_back((double)-1);
                varNames.push_back(lineValues[i]);
            }
            else if(i==1) // file name
            {
                this->vals.push_back((double)-1);
                fileNames.push_back(lineValues[i]);
            }
            else if(i==3)
                this->vals.push_back((double)lineValues[i].toLongLong(NULL,16));
            else if(i==6) // DSE
            {
                this->vals.push_back(encDepth(lineValues[i].toInt()));
                this->vals.push_back(encDirty(lineValues[i].toInt()));
                this->vals.push_back(encLocked(lineValues[i].toInt()));
                this->vals.push_back(encStlb(lineValues[i].toInt()));
            }
            else
                this->vals.push_back(lineValues[i].toDouble());
        }

        elemid++;
    }

    // Close and return
    dataFile.close();

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

    this->init();

    return 0;
}

void DataObject::calcTotalStatistics()
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
        if(!visible(elem))
            continue;

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

    // Divide by this->numVisible to get mean
    for(int i=0; i<this->numDimensions; i++)
    {
        meanValues[i] = dimSums[i] / (qreal)this->numVisible;
        for(int j=0; j<this->numDimensions; j++)
        {
            meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] /= (qreal)this->numVisible;
        }
    }

    // Covariance = E(XY) - E(X)*E(Y)
    // TODO: Possibly switch to E((x-E(x))*(y-E(y)) to avoid floating-point error
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
        if(!visible(elem))
            continue;

        for(int i=0; i<this->numDimensions; i++)
        {
            x = *(p+i);
            standardDeviations[i] += (x-meanValues[i])*(x-meanValues[i]);
        }
    }

    for(int i=0; i<this->numDimensions; i++)
    {
        standardDeviations[i] = sqrt(standardDeviations[i]/(qreal)this->numVisible);
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

void DataObject::calcSelectionStatistics()
{
    selDimSums.resize(this->numDimensions);
    selMinimumValues.resize(this->numDimensions);
    selMaximumValues.resize(this->numDimensions);
    selMeanValues.resize(this->numDimensions);
    selStandardDeviations.resize(this->numDimensions);

    selCovarianceMatrix.resize(this->numDimensions*this->numDimensions);
    selCorrelationMatrix.resize(this->numDimensions*this->numDimensions);

    selDimSums.fill(0);
    selMeanValues.fill(0);
    selStandardDeviations.fill(0);

    for(int i=0; i<this->numDimensions; i++)
    {
        selMinimumValues[i] = maximumValues[i];
        selMaximumValues[i] = minimumValues[i];
    }

    // Means and combined means
    QVector<qreal>::Iterator p;
    qreal x, y;
    long long elem;

    QVector<qreal> meanXY;
    meanXY.resize(this->numDimensions*this->numDimensions);
    meanXY.fill(0);
    for(elem=0,p=this->begin; p!=this->end; elem++,p+=this->numDimensions)
    {
        if(!selected(elem))
            continue;

        for(int i=0; i<this->numDimensions; i++)
        {
            x = *(p+i);
            selDimSums[i] += x;
            selMinimumValues[i] = min(x,selMinimumValues[i]);
            selMaximumValues[i] = max(x,selMaximumValues[i]);

            for(int j=0; j<this->numDimensions; j++)
            {
                y = *(p+j);
                meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] += x*y;
            }
        }
    }

    // Divide by this->numSelected to get mean
    for(int i=0; i<this->numDimensions; i++)
    {
        selMeanValues[i] = selDimSums[i] / (qreal)numSelected;
        for(int j=0; j<this->numDimensions; j++)
        {
            meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] /= (qreal)numSelected;
        }
    }

    // Covariance = E(XY) - E(X)*E(Y)
    // TODO: Possibly switch to E((x-E(x))*(y-E(y)) to avoid floating-point error
    for(int i=0; i<this->numDimensions; i++)
    {
        for(int j=0; j<this->numDimensions; j++)
        {
            selCovarianceMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] =
                meanXY[ROWMAJOR_2D(i,j,this->numDimensions)] - selMeanValues[i]*selMeanValues[j];
        }
    }

    // Standard deviation of each dim
    for(elem=0,p=this->begin; p!=this->end; elem++,p+=this->numDimensions)
    {
        if(!selected(elem))
            continue;

        for(int i=0; i<this->numDimensions; i++)
        {
            x = *(p+i);
            selStandardDeviations[i] += (x-selMeanValues[i])*(x-selMeanValues[i]);
        }
    }

    for(int i=0; i<this->numDimensions; i++)
    {
        selStandardDeviations[i] = sqrt(selStandardDeviations[i]/(qreal)numSelected);
    }

    // Correlation Coeff = cov(xy) / stdev(x)*stdev(y)
    for(int i=0; i<this->numDimensions; i++)
    {
        for(int j=0; j<this->numDimensions; j++)
        {
            selCorrelationMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] =
                    selCovarianceMatrix[ROWMAJOR_2D(i,j,this->numDimensions)] /
                    (selStandardDeviations[i]*selStandardDeviations[j]);
        }
    }
}


DataSetObject::DataSetObject()
{
    hw = NULL;
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
    dobj->calcTotalStatistics();
    dataObjects.push_back(dobj);
    meta = dobj->meta;
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

    FOR_EACH_DATA(setSelectionMode(mode));
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
    FOR_EACH_DATA(setHardwareTopology(hw,sel));
}

void DataSetObject::visibilityChanged()
{
    bool sel = selectionDefined();
    FOR_EACH_DATA(setHardwareTopology(hw,sel));
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

    for(int d=0; d<this->size(); d++)
    {
        SampleIdxVector *samples = &node->sampleSets[this->at(d)].first;
        for(int i=0; i<samples->size(); i++)
        {
            this->at(d)->selectData(samples->at(i));
        }
    }
}

QVector<qreal> DataSetObject::means()
{
    QVector<qreal> ret;
    ret.resize(dataObjects[0]->numDimensions);
    for(int dim=0; dim<ret.size(); dim++)
    {
        for(int d=0; d<dataObjects.size(); d++)
            ret[dim] += dataObjects[d]->sumAt(dim);
        ret[dim] /= (double)numTotal();
    }
    return ret;
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
