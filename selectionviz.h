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
    void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

public slots:
    void setDim(int v);

private:
    int dim;
};

#endif // SELECTIONVIZ_H
