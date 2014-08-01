#ifndef HARDWARETOPOLOGY_H
#define HARDWARETOPOLOGY_H

#include <QFile>
#include <QXMLStreamReader>
#include <QMap>

#include "dataobject.h"

class DataObject;
typedef QVector<int> SampleIdxVector;
typedef QPair<SampleIdxVector,int> SampleSet;

class hardwareResourceNode
{
public:
    hardwareResourceNode();

    QString name;

    int id;
    int depth;
    long long size;

    hardwareResourceNode *parent;
    QVector<hardwareResourceNode*> children;

    QMap<DataObject*,SampleSet> sampleSets;
};

class hardwareTopology
{
public:
    hardwareTopology();

    hardwareResourceNode *hardwareResourceNodeFromXMLNode(QXmlStreamReader *xml, hardwareResourceNode *parent);
    int loadHardwareTopologyFromXML(QString fileName);

    QString hardwareName;

    int numCPUs;
    int numNUMADomains;
    int totalDepth;

    hardwareResourceNode *hardwareResourceRoot;

    QVector<hardwareResourceNode*> allHardwareResourceNodes;
    QVector< QVector<hardwareResourceNode*> > hardwareResourceMatrix;

    QVector<hardwareResourceNode*> *CPUNodes;
    QVector<hardwareResourceNode*> *NUMANodes;

    QMap<int,hardwareResourceNode*> CPUIDMap;
    QMap<int,hardwareResourceNode*> NUMAIDMap;

private:
    void processLoadedTopology();
    void constructHardwareResourceMatrix();
    void addToMatrix(hardwareResourceNode *node);
};

#endif // HARDWARETOPOLOGY_H
