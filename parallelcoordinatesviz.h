#ifndef PARALLELCOORDINATESVIZ_H
#define PARALLELCOORDINATESVIZ_H

#include "vizwidget.h"

#include <QVector2D>
#include <QVector4D>

class ParallelCoordinatesViz : public VizWidget
{
    Q_OBJECT
public:
    ParallelCoordinatesViz();

public:
    void paintGL(QRect rect);
    void paint(QPainter *painter, QPaintEvent *event, int elapsed);
    void processViz();
    void recalcLines(QRect rect, int dirtyAxis = -1);

signals:

public slots:
    void setSelOpacity(int val);
    void setUnselOpacity(int val);

protected:
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void processSelection();

private:
    QRectF plotBBox;

    QVector<qreal> dimMins;
    QVector<qreal> dimMaxes;

    QVector<qreal> selMins;
    QVector<qreal> selMaxes;

    QVector<int> axesOrder;
    QVector<qreal> axesPositions;

    QPointF cursorPos;

    int selecting;
    int movingAxis;

    qreal firstSel;
    qreal lastSel;

    qreal selOpacity;
    qreal unselOpacity;

    QVector<QVector2D> linePositions;
    QVector<QVector4D> lineColors;
};

#endif // PARALLELCOORDINATESVIZ_H
