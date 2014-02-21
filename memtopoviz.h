#ifndef MEMTOPOVIZ_H
#define MEMTOPOVIZ_H

#include "vizwidget.h"

#include <QMouseEvent>
#include <QVector2D>
#include <QXmlStreamReader>
#include <QMessageBox>

class memLevel
{
public:
    QString name;
    long long id;
    long long size;

    memLevel *parent;
    QVector<memLevel*> children;

    int selected;
    QVector<int> samplesWithin;

    //QPoint center;
    int diameter;
    int height;
    float startAngle;
    float spanAngle;

    // annotations
    int numSamples;
    int totalLat;

    QColor color;
    QPoint centerPoint;

    memLevel();
    void clearSamples();
    void draw(QPainter *painter, QPoint center);
};

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
    memLevel *memLevelFromXMLNode(QXmlStreamReader *xml);
    void loadHierarchyFromXML(QString filename);
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void resizeEvent();
    void setEncDim(int dim);
    void setLatDim(int dim);
    void setCpuDim(int dim);
    void setNodeDim(int dim);

private:
    void processTopoViz();
    void processViz(QRect *bbox, memLevel *lvl, int diameter, float startAngle, float spanAngle);
    void printMemoryHierarchy(memLevel *lvl, int depth);
    memLevel* clickMemLevel(const QPoint &p);

private:
    int encDim;
    int latDim;
    int cpuDim;
    int nodeDim;

    int lvlHeight;

    int numCPUs;
    int numNodes;
    QString hardwareName;
    memLevel *memoryHierarchy;

    QColor colorBarMin;
    QColor colorBarMax;

    qreal minVal;
    qreal maxVal;

    bool topoLoaded;

    QMap<int,memLevel*> cpuMap;
    QVector<memLevel*> allLevels;
};

#endif // MEMTOPOVIZ_H
