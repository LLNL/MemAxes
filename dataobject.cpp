#include "dataobject.h"

DataObject::DataObject()
{
    // nothing
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
