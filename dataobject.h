#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QWidget>

#include "hardwaretopology.h"
#include "util.h"
#include "console.h"

class hardwareTopology;
class console;

enum select_type
{
    INVISIBLE = 0,
    VISIBLE,
    SELECTED
};

enum selection_mode
{
    L_XOR = 0,
    L_OR,
    L_AND
};

class DataObject
{
public:
    DataObject();

    selection_mode selectionMode;

public:
    void init();

    void setHardwareTopology(hardwareTopology *hw, bool sel = false);
    void selectionChanged() { setHardwareTopology(topo); }

    void setSelectionMode(selection_mode mode) { selectionMode = mode; }

    void selectData(unsigned int index);
    void deselectData(unsigned int index);
    void logicalSelectData(unsigned int index, bool select);

    void selectAllVisible();
    void deselectAll();

    void showData(unsigned int index);
    void hideData(unsigned int index);
    void showAll();
    void hideAll();

    bool visible(unsigned int index);
    bool selected(unsigned int index);

    void selectByDimRange(int dim, qreal vmin, qreal vmax);
    void selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes);
    void selectBySourceFileName(QString str);
    void selectByVarName(QString str);

    void hideSelected();
    void showSelectedOnly();

    bool selectionDefined();
    bool skip(unsigned int index);

    qreal at(int i, int d) const { return vals[i*numDimensions+d]; }
    qreal sumAt(int d) const { return dimSums[d]; }
    qreal minAt(int d) const { return minimumValues[d]; }
    qreal maxAt(int d) const { return maximumValues[d]; }
    qreal meanAt(int d) const { return meanValues[d]; }
    qreal stddevAt(int d) const { return standardDeviations[d]; }
    qreal covarianceBtwn(int d1,int d2) const { return covarianceMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }
    qreal correlationBtwn(int d1,int d2) const { return correlationMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }

    qreal selectionSumAt(int d) const { return selDimSums[d]; }
    qreal selectionMinAt(int d) const { return selMinimumValues[d]; }
    qreal selectionMaxAt(int d) const { return selMaximumValues[d]; }
    qreal selectionMeanAt(int d) const { return selMeanValues[d]; }
    qreal selectionStddevAt(int d) const { return selStandardDeviations[d]; }
    qreal selectionCovarianceBtwn(int d1,int d2) const { return selCovarianceMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }
    qreal selectionCorrelationBtwn(int d1,int d2) const { return selCorrelationMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }

    int parseCSVFile(QString dataFileName);

    void calcTotalStatistics();
    void calcSelectionStatistics();


public:
    QStringList meta;
    hardwareTopology *topo;

    long long numDimensions;
    long long numElements;
    long long numSelected;
    long long numVisible;

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
    QVector<qint8> selection;

    QVector<qreal> dimSums;
    QVector<qreal> minimumValues;
    QVector<qreal> maximumValues;
    QVector<qreal> meanValues;
    QVector<qreal> standardDeviations;
    QVector<qreal> covarianceMatrix;
    QVector<qreal> correlationMatrix;

    QVector<qreal> selDimSums;
    QVector<qreal> selMinimumValues;
    QVector<qreal> selMaximumValues;
    QVector<qreal> selMeanValues;
    QVector<qreal> selStandardDeviations;
    QVector<qreal> selCovarianceMatrix;
    QVector<qreal> selCorrelationMatrix;
};

class DataSetObject
{
public:
    DataSetObject();

    int addData(QString filename);
    int setHardwareTopology(QString filename);

    void showSelectedOnly();
    void showAll();
    void deselectAll();
    void hideSelected();
    void setSelectionMode(selection_mode mode);
    void selectAllVisible();

    bool selectionDefined();

    void selectionChanged();
    void visibilityChanged();

    void selectByMultiDimRange(QVector<int> dims, QVector<qreal> mins, QVector<qreal> maxes);

    QVector<qreal> means();
    int numSelected();
    int numUnselected();
    int numTotal();

    DataObject* at(int i) { return dataObjects[i]; }
    bool isEmpty() { return dataObjects.isEmpty(); }
    int size() { return dataObjects.size(); }
    hardwareTopology* hwTopo() { return hw; }

    void setConsole(console *icon) { con = icon; }

public:
    QStringList meta;

private:
    console *con;
    hardwareTopology *hw;
    QVector<DataObject*> dataObjects;
};

#endif // DATAOBJECT_H
