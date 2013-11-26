#ifndef PARALLELCOORDINATESVIZ_H
#define PARALLELCOORDINATESVIZ_H

#include "vizwidget.h"

#include <QVector2D>
#include <QVector4D>

class ParallelCoordinatesVizWidget
        : public VizWidget
{
    Q_OBJECT

public:
    ParallelCoordinatesVizWidget(QWidget *parent = 0);

public:
    void recalcLines(int dirtyAxis = -1);

signals:

public slots:
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void setSelOpacity(int val);
    void setUnselOpacity(int val);

protected:
    void processData();
    void paintGL();
    void drawQtPainter(QPainter *painter);

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

    QVector<GLfloat> verts;
    QVector<GLfloat> colors;
};

#endif // PARALLELCOORDINATESVIZ_H
