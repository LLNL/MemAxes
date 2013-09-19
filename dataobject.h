#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QWidget>

class DataObject
{
public:
    DataObject();

public:
    void init();

public:
    QStringList meta;

    long long numDimensions;
    long long numElements;

    QVector<qreal> vals;
    QVector<qreal>::Iterator begin;
    QVector<qreal>::Iterator end;

    QVector<qint8> selection;
};

#endif // DATAOBJECT_H
