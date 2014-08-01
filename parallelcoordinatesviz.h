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
    void lineSelected(int line);

public slots:
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void setSelOpacity(int val);
    void setUnselOpacity(int val);
    void setShowHistograms(bool checked);

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
    void calcMinMaxes();
    void calcHistBins();

private:
    int numDimensions;
    int numHistBins;

    QRectF plotBBox;
    ColorMap colorMap;

    QVector<QVector<qreal> > histVals;
    QVector<qreal> histMaxVals;

    QVector<qreal> dimMins;
    QVector<qreal> dimMaxes;

    QVector<qreal> selMins;
    QVector<qreal> selMaxes;

    QVector<int> axesOrder;
    QVector<qreal> axesPositions;

    QPointF cursorPos;

    int selecting;
    int movingAxis;

    bool showHistograms;

    qreal firstSel;
    qreal lastSel;

    qreal selOpacity;
    qreal unselOpacity;

    QVector<GLfloat> verts;
    QVector<GLfloat> colors;
};

#endif // PARALLELCOORDINATESVIZ_H
