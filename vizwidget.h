#ifndef VIZWIDGET_H
#define VIZWIDGET_H

#include <QGLWidget>

#include "dataobject.h"

class VizWidget : public QGLWidget
{
    Q_OBJECT
public:
    VizWidget(QWidget *parent = 0);
    ~VizWidget();

    QSize sizeHint() const;

signals:
    void selectionChangedSig();

public slots:
    virtual void selectionChangedSlot();

public:
    void setData(DataObject* iData);

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *event);

    virtual void processData();
    virtual void paintGL();
    virtual void drawNativeGL();
    virtual void drawQtPainter(QPainter *painter);

private:
    void beginNativeGL();
    void endNativeGL();

protected:
    DataObject *data;
    bool processed;

    int margin;
    QColor bgColor;
};

#endif // VIZWIDGET_H
