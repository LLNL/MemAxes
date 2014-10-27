#ifndef MEMTOPOVIZ_H
#define MEMTOPOVIZ_H

#include "vizwidget.h"

#include <QMouseEvent>
#include <QPair>
#include <QXmlStreamReader>
#include <QToolTip>

enum VizMode
{
    ICICLE = 0,
    SUNBURST
};

enum DataMode
{
    COLORBY_SAMPLES = 0,
    COLORBY_CYCLES
};

typedef QPair<hardwareResourceNode*,QRectF> NodeBox;
typedef QPair<QColor,QRectF> ColoredRect;

class MemTopoViz : public VizWidget
{
    Q_OBJECT
public:
    MemTopoViz(QWidget *parent = 0);

protected:
    void processData();
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

signals:

public slots:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

    void setColorByCycles(bool on) { if(on) { dataMode = COLORBY_CYCLES; selectionChangedSlot(); } }
    void setColorBySamples(bool on) { if(on) { dataMode = COLORBY_SAMPLES; selectionChangedSlot(); } }
    void setVizModeIcicle(bool on) { if(on) { vizMode = ICICLE; selectionChangedSlot(); } }
    void setVizModeSunburst(bool on) { if(on) { vizMode = SUNBURST; selectionChangedSlot(); } }

private:
    void calcMinMaxes();
    void resizeNodeBoxes();
    void zoomVertical(hardwareResourceNode *node, int dir);
    void zoomHorizontal(hardwareResourceNode *node, int dir);
    hardwareResourceNode* nodeAtPosition(QPoint p);
    void selectSamplesWithinNode(hardwareResourceNode *lvl);

private:

    QVector<NodeBox> nodeBoxes;
    QVector<ColoredRect> nodeDataBoxes;
    QVector<RealRange> depthValRanges;

    DataMode dataMode;
    VizMode vizMode;
    ColorMap colorMap;

    QVector<qreal> minCyclesPerLevel;
    QVector<qreal> maxCyclesPerLevel;

    QVector<qreal> minTransactionsPerLevel;
    QVector<qreal> maxTransactionsPerLevel;

    QVector<qreal> minSamplesPerLevel;
    QVector<qreal> maxSamplesPerLevel;

    IntRange depthRange;
    QVector<IntRange> widthRange;
};

#endif // MEMTOPOVIZ_H
