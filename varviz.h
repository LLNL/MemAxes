#ifndef VARVIZ_H
#define VARVIZ_H

#include "vizwidget.h"

struct varBlock
{
    QString name;
    qreal val;
    QRect block;
};

class VarViz : public VizWidget
{
    Q_OBJECT
public:
    VarViz(QWidget *parent = 0);
    ~VarViz();

signals:
    void variableSelected(int id);

protected:
    void processData();
    void selectionChangedSlot();
    //void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

    void mouseReleaseEvent(QMouseEvent *e);

public slots:
    void setMetricDim(int indim);
    void setVarDim(int indim);

private:
    int getVariableID(QString name);

private:
    int margin;
    int metricDim;
    int varDim;
    QRect drawSpace;

    int numVariableBlocks;

    QVector<varBlock> varBlocks;
    qreal varMaxVal;
};

#endif // VARVIZ_H
