#ifndef CORRELATIONMATRIXVIZ_H
#define CORRELATIONMATRIXVIZ_H

#include "vizwidget.h"

class CorrelationMatrixViz
        : public VizWidget
{
    Q_OBJECT

public:
    CorrelationMatrixViz(QWidget *parent = 0);

public:

signals:
    void selectedDims(int x, int y);

public slots:
    void setMin(int v);
    void setMax(int v);
    void selectionChangedSlot();

private:
    QPoint matrixID(QPoint pixel);

protected:
    void processData();
    void drawQtPainter(QPainter *painter);

    void leaveEvent(QEvent *);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QColor colorBarMin;
    QColor colorBarMax;

    qreal minVal;
    qreal maxVal;

    QRectF matrixBBox;

    int highlighted;
    int selected;

};

#endif // CORRELATIONMATRIXVIZ_H
