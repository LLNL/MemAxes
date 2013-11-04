#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QWidget>

class DataObject
{
public:
    DataObject();

public:
    void init();
    int parseCSVFile(QString dataFileName);

    qint8 selected(unsigned int index);
    void selectData(unsigned int index, qint8 sel);
    void deselectData(unsigned int index);
    void deselectAll();

    bool selectionDefined();

public:
    QStringList meta;

    long long numDimensions;
    long long numElements;

    QVector<qreal> vals;
    QVector<qreal>::Iterator begin;
    QVector<qreal>::Iterator end;

private:
    QVector<qint8> selection;
    long long numSelected;
};

#endif // DATAOBJECT_H
