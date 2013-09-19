#ifndef CORRELATIONMATRIXVIZ_H
#define CORRELATIONMATRIXVIZ_H

#include "vizwidget.h"

class CorrelationMatrixViz : public VizWidget
{
    Q_OBJECT

public:
    CorrelationMatrixViz();

public:
    void print();
    void processViz();
    void paint(QPainter *painter, QPaintEvent *event, int elapsed);

signals:
    void selectedDims(int x, int y);

public slots:
    void setMin(int v);
    void setMax(int v);

private:
    QColor valToColor(qreal val);
    QPoint matrixID(QPoint pixel);

protected:
    void leaveEvent(QEvent *);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QVector<qreal> standardDeviations;
    QVector<qreal> expectedValues;
    QVector<qreal> covarianceMatrix;
    QVector<qreal> correlationMatrix;

    QColor colorBarMin;
    QColor colorBarMax;

    qreal minVal;
    qreal maxVal;

    QRectF matrixBBox;

    int highlighted;
    int selected;

};

#endif // CORRELATIONMATRIXVIZ_H
