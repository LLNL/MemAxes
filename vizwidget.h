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
    void visibilityChangedSig();

public slots:
    virtual void selectionChangedSlot();
    virtual void visibilityChangedSlot();

public:
    void setDataSet(DataSetObject *iDataSet);
    void setConsole(console *iCon);
    virtual void processData();

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *event);

    virtual void paintGL();
    virtual void drawNativeGL();
    virtual void drawQtPainter(QPainter *painter);

private:
    void beginNativeGL();
    void endNativeGL();

protected:
    bool processed;
    console *con;
    DataSetObject *dataSet;

    int margin;
    QColor bgColor;
};

#endif // VIZWIDGET_H
