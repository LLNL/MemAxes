#ifndef SELECTIONVIZ_H
#define SELECTIONVIZ_H

#include "vizwidget.h"

#include <QSpinBox>

class SelectionVizWidget : public VizWidget
{
    Q_OBJECT
public:
    SelectionVizWidget(QWidget *parent = 0);

signals:

protected:
    void processData();
    void selectionChangedSlot();
    void drawQtPainter(QPainter *painter);

public slots:
    void setDim(int v);

private:
    QVector<qreal> mins;
    QVector<qreal> maxes;
    QVector<qreal> totals;
    QVector<qreal> means;
    QVector<qreal> stddevs;

    QVector<qreal> selectionMins;
    QVector<qreal> selectionMaxes;
    QVector<qreal> selectionTotals;
    QVector<qreal> selectionMeans;
    QVector<qreal> selStddevs;

    int dim;
};

#endif // SELECTIONVIZ_H
