#include "dataobject.h"

#include <iostream>
using namespace std;

#include <QFile>
#include <QTextStream>

DataObject::DataObject()
{
    numSelected = 0;
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

        for(int i=0; i<lineValues.size(); i++)
        {
            this->vals.push_back(lineValues[i].toDouble());
        }

        elemid++;
    }

    // Close and return
    dataFile.close();

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

void DataObject::deselectAll()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(selected(elem))
            deselectData(elem);
    }
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

void DataObject::filterByDimRange(int dim, qreal vmin, qreal vmax)
{
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        if(*(p+dim) >= vmin && *(p+dim) <= vmax)
            this->showData(elem);
    }
}

void DataObject::selectByDimRange(int dim, qreal vmin, qreal vmax)
{
    long long elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=this->begin; p!=this->end; elem++, p+=this->numDimensions)
    {
        if(*(p+dim) >= vmin && *(p+dim) < vmax)
            this->selectData(elem);
    }
}

void DataObject::filterBySelection()
{
    long long elem;
    for(elem=0; elem<numElements; elem++)
    {
        if(selected(elem))
            deselectData(elem);
        else if(visible(elem))
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

