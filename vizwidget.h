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
    virtual void paintGL(QRect rect);
    virtual void paint(QPainter *painter, QPaintEvent *event, int elapsed);
    virtual void processViz();

signals:
    void selectionChangedSig();

public slots:
    virtual void selectionChangedSlot();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeGL(int width, int height);

protected:
    DataObject *data;
    bool vizProcessed;
    QBrush backgroundColor;
    QBrush selectColor;
    QRect winRect;
};

#endif // VIZWIDGET_H
