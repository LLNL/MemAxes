#ifndef VIZWIDGET_H
#define VIZWIDGET_H

#include <QGLWidget>

#include "dataobject.h"

class VizWidget : public QGLWidget
{
    Q_OBJECT
public:
    VizWidget();

public:
    void setData(DataObject *d) { data = d; }
    virtual void paint(QPainter *painter, QPaintEvent *event, int elapsed);
    virtual void processViz();

signals:
    void repaintAll();

public slots:

protected:
    void paintEvent(QPaintEvent *event);

protected:
    DataObject *data;
    bool vizProcessed;
    QBrush backgroundColor;
    QBrush selectColor;
};

#endif // VIZWIDGET_H
