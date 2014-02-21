#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QWidget>

#include "util.h"

enum select_type
{
    INVISIBLE = 0,
    VISIBLE,
    SELECTED
};

class DataObject
{
public:
    DataObject();

public:
    void init();
    int parseCSVFile(QString dataFileName);

    void calcTotalStatistics();
    void calcSelectionStatistics();

    void selectData(unsigned int index);
    void deselectData(unsigned int index);
    void deselectAll();

    void showData(unsigned int index);
    void showAll();
    void hideData(unsigned int index);
    void hideAll();

    bool visible(unsigned int index);
    bool selected(unsigned int index);

    void filterByDimRange(int dim, qreal vmin, qreal vmax);
    void selectByDimRange(int dim, qreal vmin, qreal vmax);
    void selectBySourceFileName(QString str);
    void selectByVarName(QString str);

    void filterBySelection();

    bool selectionDefined();

    qreal sumAt(int d) const { return dimSums[d]; }
    qreal minAt(int d) const { return minimumValues[d]; }
    qreal maxAt(int d) const { return maximumValues[d]; }
    qreal meanAt(int d) const { return meanValues[d]; }
    qreal medianAt(int d) const { return medianValues[d]; }
    qreal modeAt(int d) const { return modeValues[d]; }
    qreal stddevAt(int d) const { return standardDeviations[d]; }
    qreal covarianceBtwn(int d1,int d2) const { return covarianceMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }
    qreal correlationBtwn(int d1,int d2) const { return correlationMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }

    qreal selectionSumAt(int d) const { return selDimSums[d]; }
    qreal selectionMinAt(int d) const { return selMinimumValues[d]; }
    qreal selectionMaxAt(int d) const { return selMaximumValues[d]; }
    qreal selectionMeanAt(int d) const { return selMeanValues[d]; }
    qreal selectionMedianAt(int d) const { return selMedianValues[d]; }
    qreal selectionModeAt(int d) const { return selModeValues[d]; }
    qreal selectionStddevAt(int d) const { return selStandardDeviations[d]; }
    qreal selectionCovarianceBtwn(int d1,int d2) const { return selCovarianceMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }
    qreal selectionCorrelationBtwn(int d1,int d2) const { return selCorrelationMatrix[ROWMAJOR_2D(d1,d2,numDimensions)]; }

public:
    QStringList meta;

    long long numDimensions;
    long long numElements;
    long long numSelected;
    long long numVisible;

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
    QVector<qreal> medianValues; // TODO
    QVector<qreal> modeValues; // TODO
    QVector<qreal> standardDeviations;
    QVector<qreal> covarianceMatrix;
    QVector<qreal> correlationMatrix;

    QVector<qreal> selDimSums;
    QVector<qreal> selMinimumValues;
    QVector<qreal> selMaximumValues;
    QVector<qreal> selMeanValues;
    QVector<qreal> selMedianValues; // TODO
    QVector<qreal> selModeValues; // TODO
    QVector<qreal> selStandardDeviations;
    QVector<qreal> selCovarianceMatrix;
    QVector<qreal> selCorrelationMatrix;
};

#endif // DATAOBJECT_H
