#ifndef MEMTOPOVIZ_H
#define MEMTOPOVIZ_H

#include "vizwidget.h"

#include <QMouseEvent>
#include <QVector2D>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QToolTip>

enum colorMode
{
    COLORBY_SAMPLES = 0,
    COLORBY_CYCLES
};

class memLevel
{
public:
    QString name;
    long long id;
    long long depth;
    long long size;

    memLevel *parent;
    QVector<memLevel*> children;

    int selected;
    QVector<int> samplesWithin;
    QVector<int> transactions;

    float radius;
    float thickness;
    float startAngle;
    float spanAngle;
    QPoint circleCenter;
    QVector<QPointF> polygon;

    // annotations
    float numSelectedCycles;
    float numSelectedSamples;
    float numSelectedTransactions;

    QColor color;
    QPointF midPoint;

    memLevel();

    void constructPoly();
};

class MemTopoViz : public VizWidget
{
    Q_OBJECT
public:
    MemTopoViz(QWidget *parent = 0);

protected:
    void processData();
    void processSelection();
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void drawSelectionPie(QPainter *painter, QRect bbox);
    void drawQtPainter(QPainter *painter);

signals:

public slots:
    memLevel *memLevelFromXMLNode(QXmlStreamReader *xml);
    void loadHierarchyFromXML(QString filename);
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    void setEncDim(int dim);
    void setLatDim(int dim);
    void setCpuDim(int dim);
    void setNodeDim(int dim);

    void setColorByCycles(bool on) { if(on) { mode = COLORBY_CYCLES; repaint(); } }
    void setColorBySamples(bool on) { if(on) { mode = COLORBY_SAMPLES; repaint(); } }

private:
    void processTopoViz();
    void processViz(QRect *bbox, memLevel *lvl, int radius, float startAngle, float spanAngle, int depth);
    void printMemoryHierarchy(memLevel *lvl, int depth);
    memLevel* levelAtPosition(QPoint p);
    void selectLevel(memLevel *lvl);

private:
    colorMode mode;

    int encDim;
    int latDim;
    int cpuDim;
    int nodeDim;

    int lvlHeight;
    int totalDepth;

    int numCPUs;
    int numNodes;
    QString hardwareName;
    memLevel *memoryHierarchy;

    QColor colorBarMin;
    QColor colorBarMax;

    QVector<qreal> minCyclesPerLevel;
    QVector<qreal> maxCyclesPerLevel;

    QVector<qreal> minTransactionsPerLevel;
    QVector<qreal> maxTransactionsPerLevel;

    QVector<qreal> minSamplesPerLevel;
    QVector<qreal> maxSamplesPerLevel;

    bool topoLoaded;

    QMap<int,memLevel*> cpuMap;
    QVector<memLevel*> allLevels;
};

#endif // MEMTOPOVIZ_H
