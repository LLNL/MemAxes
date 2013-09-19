#ifndef PARALLELCOORDINATESVIZ_H
#define PARALLELCOORDINATESVIZ_H

#include "vizwidget.h"

class ParallelCoordinatesViz : public VizWidget
{
    Q_OBJECT
public:
    ParallelCoordinatesViz();

public:
    void paint(QPainter *painter, QPaintEvent *event, int elapsed);
    void processViz();

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

    QVector<QLineF> pointLines;

    QPointF cursorPos;

    int selecting;
    qreal firstSel;
    qreal lastSel;

    qreal selOpacity;
    qreal unselOpacity;
};

#endif // PARALLELCOORDINATESVIZ_H
