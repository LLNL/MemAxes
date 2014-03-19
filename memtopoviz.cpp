#include "memtopoviz.h"

#include <iostream>
#include <cmath>

using namespace std;

MemTopoViz::MemTopoViz(QWidget *parent) :
    VizWidget(parent)
{
    mode = COLORBY_CYCLES;

    colorBarMin = QColor(254,230,206);
    colorBarMax = QColor(230,85,13);

    memoryHierarchy = NULL;
    topoLoaded = false;

    encDim = 0;
    latDim = 0;
    cpuDim = 0;
    nodeDim = 0;

    totalDepth = 0;
    lvlHeight = 20;

    numCPUs = 0;

    this->installEventFilter(this);
    setMouseTracking(true);
}

void MemTopoViz::processData()
{
    if(!topoLoaded)
        return;

    data->calcTotalStatistics();

    // Clear annotations
    for(int i=0; i<allLevels.size(); i++)
    {
        allLevels[i]->samplesWithin.clear();
        allLevels[i]->transactions.clear();
    }

    // Annotate levels with samples
    int elem;
    QVector<qreal>::Iterator p;
    for(elem=0, p=data->begin; p!=data->end; elem++, p+=data->numDimensions)
    {
        // Parse sample
        int enc = *(p+encDim);
        int cpu = *(p+cpuDim);

        // Find cpu
        memLevel *lvl = cpuMap[cpu];
        lvl->samplesWithin.push_back(elem);

        int sampleDepth = enc;

        if(sampleDepth < 0)
            continue;

        // annotate up
        while(sampleDepth > 0)
        {
            lvl->transactions.push_back(elem);
            lvl = lvl->parent;
            sampleDepth--;
        }

        lvl->samplesWithin.push_back(elem);
    }

    processed = true;

    processSelection();
}

void MemTopoViz::processSelection()
{
    data->calcSelectionStatistics();

    minCyclesPerLevel.fill(std::numeric_limits<qreal>::max());
    minTransactionsPerLevel.fill(std::numeric_limits<qreal>::max());
    minSamplesPerLevel.fill(std::numeric_limits<qreal>::max());

    maxCyclesPerLevel.fill(0);
    maxTransactionsPerLevel.fill(0);
    maxSamplesPerLevel.fill(0);

    bool selectionDefined = data->selectionDefined();
    for(int i=0; i<allLevels.size(); i++)
    {
        int depth = allLevels[i]->depth;

        allLevels[i]->numSelectedCycles = 0;
        allLevels[i]->numSelectedSamples = 0;
        allLevels[i]->numSelectedTransactions = 0;

        for(int s=0; s<allLevels[i]->samplesWithin.size(); s++)
        {
            if(!selectionDefined || data->selected(allLevels[i]->samplesWithin[s]))
            {
                allLevels[i]->numSelectedCycles += data->at(allLevels[i]->samplesWithin[s],latDim);
                allLevels[i]->numSelectedSamples += 1;
            }
        }

        for(int s=0; s<allLevels[i]->transactions.size(); s++)
        {
            if(!selectionDefined || data->selected(allLevels[i]->transactions[s]))
            {
                allLevels[i]->numSelectedTransactions += 1;
            }
        }

        minCyclesPerLevel[depth] = fmin(minCyclesPerLevel[depth], allLevels[i]->numSelectedCycles);
        minTransactionsPerLevel[depth] = fmin(minTransactionsPerLevel[depth], allLevels[i]->numSelectedTransactions);
        minSamplesPerLevel[depth] = fmin(minSamplesPerLevel[depth], allLevels[i]->numSelectedSamples);

        maxCyclesPerLevel[depth] = fmax(maxCyclesPerLevel[depth], allLevels[i]->numSelectedCycles);
        maxTransactionsPerLevel[depth] = fmax(maxTransactionsPerLevel[depth], allLevels[i]->numSelectedTransactions);
        maxSamplesPerLevel[depth] = fmax(maxSamplesPerLevel[depth], allLevels[i]->numSelectedSamples);
    }
}

void MemTopoViz::selectionChangedSlot()
{
    if(!data->selectionDefined())
        for(int i=0; i<allLevels.size(); i++)
            allLevels[i]->selected = 0;

    processSelection();
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

    painter->setBrush(bgColor);
    painter->drawRect(rect());

    // Draw transactions
    painter->setBrush(Qt::gray);
    painter->setPen(Qt::NoPen);

    memLevel interconnect;
    interconnect.circleCenter = rect().center();

    for(int i=0; i<allLevels.size(); i++)
    {
        int depth = allLevels[i]->depth;

        if(depth < 2)
            continue;

        float midAngle = allLevels[i]->startAngle +
                         allLevels[i]->spanAngle/2;

        float thickness = scale(allLevels[i]->numSelectedTransactions,
                                minTransactionsPerLevel[depth],maxTransactionsPerLevel[depth],
                                0.01,1);

        float angleThickness = thickness*allLevels[i]->spanAngle;

        interconnect.startAngle = midAngle-angleThickness/2;
        interconnect.spanAngle = angleThickness;
        interconnect.radius = allLevels[i]->radius - allLevels[i]->thickness;
        interconnect.thickness = allLevels[i]->thickness;

        interconnect.constructPoly();

        painter->drawPolygon(interconnect.polygon.constData(),interconnect.polygon.size());
    }

    // Draw level segments
    for(int i=0; i<allLevels.size(); i++)
    {
        int depth = allLevels[i]->depth;

        if(depth == 0)
            continue;

        if(depth == totalDepth-1)
        {
            painter->setBrush(valToColor(allLevels[i]->numSelectedTransactions,
                                         minTransactionsPerLevel[depth],maxTransactionsPerLevel[depth],
                                         colorBarMin,colorBarMax));
        }
        else
        {
            if(mode == COLORBY_CYCLES)
            {
                if(maxCyclesPerLevel[depth] == 0)
                    painter->setBrush(colorBarMin);
                else
                    painter->setBrush(valToColor(allLevels[i]->numSelectedCycles,
                                                 minCyclesPerLevel[depth],maxCyclesPerLevel[depth],
                                                 colorBarMin,colorBarMax));
            }
            else if(mode == COLORBY_SAMPLES)
            {
                if(maxSamplesPerLevel[depth] == 0)
                    painter->setBrush(colorBarMin);
                else
                    painter->setBrush(valToColor(allLevels[i]->numSelectedSamples,
                                                 minSamplesPerLevel[depth],maxSamplesPerLevel[depth],
                                                 colorBarMin,colorBarMax));
            }
            else
            {
                cerr << "PARAKEETS" << endl;
                return;
            }
        }


        if(allLevels[i]->selected)
            painter->setPen(QPen(Qt::yellow,2));
        else
            painter->setPen(QPen(Qt::black,2));

        painter->drawPolygon(allLevels[i]->polygon.constData(),allLevels[i]->polygon.size());
    }

    // Draw id numbers
    painter->setPen(QPen(Qt::black));
    for(int i=0; i<allLevels.size(); i++)
    {
        if(allLevels[i]->depth == 0)
            continue;

        painter->save();
        painter->translate(allLevels[i]->midPoint);
        painter->rotate(90-(allLevels[i]->startAngle+allLevels[i]->spanAngle/2)/16);
        painter->drawText(0,0,QString::number(allLevels[i]->id));
        painter->restore();
    }

    // Draw selection pie
    //int margin = 10;
    //int selSide = 150;
    //QRect selBox(rect().right()-selSide-margin,rect().top()+margin,selSide-margin,selSide-margin);
    //drawSelectionPie(painter,selBox);
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
    else
    {
        //newLevel->transactions.resize(newLevel->children.size());
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

    memLevel *lvl = levelAtPosition(e->pos());

    if(lvl)
        selectLevel(lvl);

    repaint();
}

void MemTopoViz::wheelEvent(QWheelEvent *e)
{
    lvlHeight += e->delta()/80;
    processTopoViz();
    repaint();
}

void MemTopoViz::mouseMoveEvent(QMouseEvent* e)
{
    memLevel *lvl = levelAtPosition(e->pos());

    if(lvl)
    {
        QString label;

        if(lvl->depth == totalDepth-1)
            label = "CPU " + QString::number(lvl->id) + "\n";
        else if(lvl->depth > 1)
            label = "L" + QString::number(lvl->id) + " Cache\n";
        else if(lvl->depth == 1)
            label = "NUMA Node " + QString::number(lvl->id) + "\n";
        else
            label = "RAM\n";

        label += "\n";
        label += "Size: " + QString::number(lvl->size) + " bytes\n";

        label += "\n";
        label += "Cycles: " + QString::number(lvl->numSelectedCycles) + "\n";
        label += "Accesses: " + QString::number(lvl->numSelectedSamples) + "\n";
        label += "Transactions: " + QString::number(lvl->numSelectedTransactions) + "\n";

        label += "\n";
        label += "Cycles/Access: " + QString::number(lvl->numSelectedCycles/lvl->numSelectedSamples) + "\n";

        QToolTip::showText(e->globalPos(),label,this, rect() );
    }
    else
    {
        QToolTip::hideText();
    }
}

void MemTopoViz::resizeEvent(QResizeEvent *e)
{
    VizWidget::resizeEvent(e);
    repaint();
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
    processViz(&bbox,memoryHierarchy,lvlHeight,0,16*360,0);

    minCyclesPerLevel.resize(totalDepth);
    maxCyclesPerLevel.resize(totalDepth);

    minTransactionsPerLevel.resize(totalDepth);
    maxTransactionsPerLevel.resize(totalDepth);

    minSamplesPerLevel.resize(totalDepth);
    maxSamplesPerLevel.resize(totalDepth);
}

void MemTopoViz::processViz(QRect *bbox, memLevel *lvl, int radius, float startAngle, float spanAngle, int depth)
{
    int thickness = lvlHeight;
    int separation = lvlHeight;

    if(!topoLoaded)
        return;

    float deltaAngle = spanAngle;
    if(!lvl->children.isEmpty())
        deltaAngle /= lvl->children.size();

    for(int i=0; i<lvl->children.size(); i++)
        processViz(bbox, lvl->children[i], radius+thickness+separation, startAngle+i*deltaAngle, deltaAngle, depth+1);

    lvl->circleCenter = bbox->center();
    lvl->radius = radius;
    lvl->startAngle = startAngle+20;
    lvl->spanAngle = spanAngle-40;
    lvl->thickness = thickness;
    lvl->depth = depth;

    lvl->midPoint = lvl->circleCenter +
                    polarToCartesian(lvl->radius - lvl->thickness/2 - 4,
                                     lvl->startAngle + lvl->spanAngle/2 + 12);
    lvl->color = Qt::gray;

    if(totalDepth < depth+1)
        totalDepth = depth+1;

    lvl->constructPoly();
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
              << lvl->numSelectedCycles << std::endl;

    for(int i=0; i<lvl->children.size(); i++)
    {
        printMemoryHierarchy(lvl->children[i],depth+1);
    }
}

memLevel *MemTopoViz::levelAtPosition(QPoint p)
{
    QPointF polarMouse = cartesianToPolar(p - rect().center());
    float mag = polarMouse.x();
    float theta = polarMouse.y();

    for(int i=0; i<allLevels.size(); i++)
    {
        bool inAngle = within(theta,
                              allLevels[i]->startAngle,
                              allLevels[i]->startAngle+allLevels[i]->spanAngle);

        bool inMag = within(mag,
                            allLevels[i]->radius-allLevels[i]->thickness,
                            allLevels[i]->radius);

        if(inAngle && inMag)
            return allLevels[i];
    }

    return NULL;
}

void MemTopoViz::selectLevel(memLevel *lvl)
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

memLevel::memLevel()
    :
      name("unnamed"),
      id(0),
      size(0),
      parent(NULL),
      selected(0),
      numSelectedCycles(0),
      numSelectedSamples(0),
      numSelectedTransactions(0)
{

}

void memLevel::constructPoly()
{
    polygon.clear();

    int degreesPerPoint = 5;
    int arcPoints = (spanAngle / 16) / degreesPerPoint;
    float angle = startAngle;
    float deltaAngle = spanAngle / (float)arcPoints;
    for(int i=0; i<=arcPoints; i++)
    {
        polygon.push_back(circleCenter+polarToCartesian(radius,angle));
        angle += deltaAngle;
    }
    angle -= deltaAngle;

    for(int i=0; i<=arcPoints; i++)
    {
        polygon.push_back(circleCenter+polarToCartesian(radius-thickness,angle));
        angle -= deltaAngle;
    }
}
