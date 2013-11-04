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
    selection.fill(0);
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

qint8 DataObject::selected(unsigned int index)
{
    return selection[index];
}

void DataObject::selectData(unsigned int index, qint8 sel)
{
    if(selection[index] == 0)
    {
        selection[index] = sel;
        numSelected++;
    }
}

void DataObject::deselectData(unsigned int index)
{
    if(selection[index] != 0)
    {
        selection[index] = 0;
        numSelected--;
    }
}

void DataObject::deselectAll()
{
    selection.fill(0);
}

bool DataObject::selectionDefined()
{
    return numSelected > 0;
}

