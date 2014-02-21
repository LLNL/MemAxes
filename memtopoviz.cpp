#include "memtopoviz.h"

#include <iostream>
#include <cmath>

MemTopoViz::MemTopoViz(QWidget *parent) :
    VizWidget(parent)
{
    minVal = 0;
    maxVal = 0;

    colorBarMin = QColor(0,0,255);
    colorBarMax = QColor(255,0,0);

    memoryHierarchy = NULL;
    topoLoaded = false;

    encDim = 0;
    latDim = 0;
    cpuDim = 0;
    nodeDim = 0;

    lvlHeight = 50;

    numCPUs = 0;

    this->installEventFilter(this);
}

void MemTopoViz::processData()
{
    if(!topoLoaded)
        return;

    // Clear annotations
    memoryHierarchy->clearSamples();
    maxVal = 0;

    // Annotate levels with samples
    int elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=data->begin; p!=data->end; elem++, p+=data->numDimensions)
    {
        if(data->selectionDefined() && !data->selected(elem))
            continue;

        // Parse sample
        int enc = *(p+encDim);
        int cpu = *(p+cpuDim);
        int lat = *(p+latDim);

        memLevel *lvl = cpuMap[cpu];
        lvl->samplesWithin.push_back(elem);

        int sampleDepth = enc;

        //std::cout << "sampleDepth : " << sampleDepth << std::endl;

        if(sampleDepth < 0)
            continue;

        while(lvl && sampleDepth > 0)
        {
            lvl = lvl->parent;
            sampleDepth--;
        }

        lvl->samplesWithin.push_back(elem);
        lvl->numSamples += lat;
        maxVal = fmax(maxVal,lvl->numSamples);
    }

    //printMemoryHierarchy(memoryHierarchy,0);

    processed = true;
}

void MemTopoViz::selectionChangedSlot()
{
    if(!data->selectionDefined())
    {
        for(int i=0; i<allLevels.size(); i++)
            allLevels[i]->selected = 0;
    }

    processData();
    repaint();
}

void MemTopoViz::visibilityChangedSlot()
{
    repaint();
}

void MemTopoViz::drawQtPainter(QPainter *painter)
{
    if(!topoLoaded)
        return;

    // pie chart
    for(int i=0; i<allLevels.size(); i++)
    {
        painter->setBrush(QBrush(valToColor(allLevels[i]->numSamples,minVal,maxVal,colorBarMin,colorBarMax)));
        allLevels[i]->draw(painter,rect().center());
    }

    painter->setBrush(QColor(0,0,0));
    for(int i=0; i<allLevels.size(); i++)
    {
        painter->save();
        painter->translate(allLevels[i]->centerPoint);
        //painter->rotate(allLevels[i]->startAngle);
        painter->drawText(0,0,QString::number(allLevels[i]->id));
        painter->restore();
    }
}

memLevel* MemTopoViz::memLevelFromXMLNode(QXmlStreamReader *xml)
{
    memLevel *newLevel = new memLevel();

    // Store parent
    newLevel->name = xml->name().toString(); // Hardware, NUMA, Cache, CPU

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
            memLevel *child = memLevelFromXMLNode(xml);
            child->parent = newLevel;

            if(child)
                newLevel->children.push_back(child);
        }

        xml->readNext();
    }

    allLevels.push_back(newLevel);

    if(newLevel->children.isEmpty())
    {
        cpuMap[newLevel->id] = newLevel;
        numCPUs++;
    }

    return newLevel;
}

void MemTopoViz::loadHierarchyFromXML(QString filename)
{
    QFile* file = new QFile(filename);

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,
                              "MemTopoViz::loadHierarchyFromXML",
                              "Couldn't open example.xml",
                              QMessageBox::Ok);
        return;
    }

    QXmlStreamReader xml(file);
    while(!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if(xml.isStartDocument())
            continue;

        if(xml.isStartElement())
            if(xml.name() == "Hardware")
                memoryHierarchy = memLevelFromXMLNode(&xml);
    }

    if(xml.hasError()) {
        QMessageBox::critical(this,
                              "MemTopoViz::loadHierarchyFromXML",
                              xml.errorString(),
                              QMessageBox::Ok);
    }

    xml.clear();

    numNodes = memoryHierarchy->children.size();

    topoLoaded = true;
    processTopoViz();

    repaint();
}

void MemTopoViz::mousePressEvent(QMouseEvent *e)
{
    if(!processed)
        return;

    memLevel *lvl = clickMemLevel(e->pos());

    if(lvl)
    {

        if(!lvl->selected)
        {
            lvl->selected = 1;
            for(int i=0; i<lvl->samplesWithin.size(); i++)
                data->selectData(lvl->samplesWithin[i]);
        }
        else
        {
            lvl->selected = 0;
            for(int i=0; i<lvl->samplesWithin.size(); i++)
                data->deselectData(lvl->samplesWithin[i]);
        }
        emit selectionChangedSig();
    }

    repaint();
}

void MemTopoViz::wheelEvent(QWheelEvent *e)
{
    lvlHeight += e->delta()/40;
    processTopoViz();
    repaint();
}

void MemTopoViz::resizeEvent()
{
    processTopoViz();
}

void MemTopoViz::setEncDim(int dim)
{
    encDim = dim;
}

void MemTopoViz::setLatDim(int dim)
{
    latDim = dim;
}

void MemTopoViz::setCpuDim(int dim)
{
    cpuDim = dim;
}

void MemTopoViz::setNodeDim(int dim)
{
    nodeDim = dim;
}

void MemTopoViz::processTopoViz()
{
    if(!topoLoaded)
        return;

    QRect bbox = rect();
    processViz(&bbox,memoryHierarchy,lvlHeight,0,16*360);
}

void MemTopoViz::processViz(QRect *bbox, memLevel *lvl, int diameter, float startAngle, float spanAngle)
{
    int defHeight = lvlHeight;

    if(!topoLoaded)
        return;

    float deltaAngle = spanAngle;
    if(!lvl->children.isEmpty())
        deltaAngle /= lvl->children.size();

    for(int i=0; i<lvl->children.size(); i++)
        processViz(bbox, lvl->children[i], diameter+defHeight, startAngle+i*deltaAngle, deltaAngle);

    //lvl->center = bbox->center();
    lvl->diameter = diameter;
    lvl->startAngle = startAngle;
    lvl->spanAngle = spanAngle;
    lvl->height = defHeight;

    float midAngle = startAngle;
    float midDiameter = (diameter-defHeight/2)/2;
    float xpos = midDiameter * cos(midAngle);
    float ypos = midDiameter * sin(midAngle);
    lvl->centerPoint = bbox->center()+QPoint(xpos,ypos);
    lvl->color = Qt::gray;
}

void MemTopoViz::printMemoryHierarchy(memLevel *lvl, int depth)
{
    for(int i=0; i<depth; i++)
    {
        std::cout << "\t";
    }

    std::cout << lvl->name.toStdString() << ", "
              << lvl->id << ", "
              << lvl->size  << ", samples: "
              << lvl->numSamples << std::endl;

    for(int i=0; i<lvl->children.size(); i++)
    {
        printMemoryHierarchy(lvl->children[i],depth+1);
    }
}

memLevel *MemTopoViz::clickMemLevel(const QPoint &p)
{
    // top/bottom
    QVector2D v(p - rect().center());
    float mag = v.length();
    float theta = atan(fabs(v.y()/v.x()));
    bool xpos = (v.x()<0) ? false : true;
    bool ypos = (v.y()<0) ? false : true;

    // Radians to degrees
    theta = 360*(theta/(2*3.14159));

    // quadrants
    if(!xpos && !ypos)
        theta = 180+theta;
    if(!xpos && ypos)
        theta = 180-theta;
    if(xpos && !ypos)
        theta = 360-theta;

    // flip to match rings
    theta = 360-theta;

    // wtf qt?
    theta *= 16;

    for(int i=0; i<allLevels.size(); i++)
    {
        bool magTest = allLevels[i]->diameter/2 > mag
                && allLevels[i]->diameter/2-allLevels[i]->height/2 < mag;
        bool radTest = allLevels[i]->startAngle < theta
                && allLevels[i]->startAngle + allLevels[i]->spanAngle > theta;

        if(magTest && radTest)
            return allLevels[i];
    }

    return NULL;
}

memLevel::memLevel()
    :
      name("unnamed"),
      id(0),
      size(0),
      parent(NULL),
      selected(0),
      numSamples(0)
{

}

void memLevel::clearSamples()
{
    numSamples = 0;
    for(int i=0; i<children.size(); i++)
    {
        children[i]->clearSamples();
    }

}

void memLevel::draw(QPainter *painter,QPoint center)
{
    if(selected)
        painter->setPen(QPen(Qt::yellow, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else
        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    painter->drawPie(center.x()-diameter/2, center.y()-diameter/2,
                     diameter, diameter,
                     startAngle, spanAngle);
    //int smallDiameter = diameter-height;

    //painter->setBrush(QBrush(Qt::white));
    //painter->drawPie(center.x()-smallDiameter/2, center.y()-smallDiameter/2,
    //                 smallDiameter, smallDiameter,
    //                 startAngle, spanAngle);
}
