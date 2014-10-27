#include "hardwaretopology.h"

#include <iostream>
using namespace std;

hardwareResourceNode::hardwareResourceNode()
{
}

hardwareTopology::hardwareTopology()
{
    numCPUs = 0;
    numNUMADomains = 0;
    totalDepth = 0;

    hardwareResourceRoot = NULL;
}

hardwareResourceNode *hardwareTopology::hardwareResourceNodeFromXMLNode(QXmlStreamReader *xml, hardwareResourceNode *parent)
{
    hardwareResourceNode *newLevel = new hardwareResourceNode();

    newLevel->parent = parent;
    newLevel->name = xml->name().toString(); // Hardware, NUMA, Cache, CPU
    newLevel->depth = (parent == NULL) ? 0 : parent->depth + 1;

    totalDepth = max(totalDepth, newLevel->depth);

    if(xml->attributes().hasAttribute("id"))
        newLevel->id = xml->attributes().value("id").toString().toLongLong();

    if(xml->attributes().hasAttribute("size"))
        newLevel->size = xml->attributes().value("size").toString().toLongLong();

    // Read and add children if existent
    xml->readNext();

    while(!(xml->isEndElement() && xml->name() == newLevel->name))
    {
        if(xml->isStartElement())
        {
            hardwareResourceNode *child = hardwareResourceNodeFromXMLNode(xml, newLevel);
            newLevel->children.push_back(child);
        }

        xml->readNext();
    }

    return newLevel;
}

int hardwareTopology::loadHardwareTopologyFromXML(QString fileName)
{
    QFile* file = new QFile(fileName);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QXmlStreamReader xml(file);
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if(xml.isStartDocument())
            continue;

        if(xml.isStartElement())
            if(xml.name() == "Hardware")
                hardwareResourceRoot = hardwareResourceNodeFromXMLNode(&xml,NULL);
    }

    if(xml.hasError()) {
        return -1;
    }

    file->close();
    xml.clear();

    processLoadedTopology();

    return 0;
}

void hardwareTopology::processLoadedTopology()
{
    allHardwareResourceNodes.clear();
    hardwareResourceMatrix.clear();

    CPUNodes = NULL;
    NUMANodes = NULL;

    CPUIDMap.clear();
    NUMAIDMap.clear();

    constructHardwareResourceMatrix();

    NUMANodes = &hardwareResourceMatrix[1];
    CPUNodes = &hardwareResourceMatrix[totalDepth];

    numNUMADomains = NUMANodes->size();
    numCPUs = CPUNodes->size();

    // Map NUMA IDs to NUMA Nodes
    for(int i=0; i<NUMANodes->size(); i++)
        NUMAIDMap[NUMANodes->at(i)->id] = NUMANodes->at(i);

    // Map CPU IDs to CPU Nodes
    for(int i=0; i<CPUNodes->size(); i++)
        CPUIDMap[CPUNodes->at(i)->id] = CPUNodes->at(i);

}

void hardwareTopology::addToMatrix(hardwareResourceNode *node)
{
    hardwareResourceMatrix[node->depth].push_back(node);
    allHardwareResourceNodes.push_back(node);

    if(node->children.empty())
        return;

    for(int i=0; i<node->children.size(); i++)
    {
        addToMatrix(node->children[i]);
    }

}

void hardwareTopology::constructHardwareResourceMatrix()
{
   hardwareResourceMatrix.resize(totalDepth+1);
   addToMatrix(hardwareResourceRoot);
}
