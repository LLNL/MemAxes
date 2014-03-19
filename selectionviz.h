#ifndef SELECTIONVIZ_H
#define SELECTIONVIZ_H

#include "vizwidget.h"

#include <QSpinBox>

enum weightMode
{
    WEIGHTBY_SAMPLES = 0,
    WEIGHTBY_CYCLES
};

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
    void setWeightModeSamples(bool on) { if(on) { mode = WEIGHTBY_SAMPLES; repaint();} }
    void setWeightModeLatency(bool on) { if(on) { mode = WEIGHTBY_CYCLES; repaint();} }

private:
    weightMode mode;

};

#endif // SELECTIONVIZ_H
