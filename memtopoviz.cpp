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

    minScale = 0;

    totalDepth = 0;
    lvlHeight = 20;

    numCPUs = 0;

    this->installEventFilter(this);
    setMouseTracking(true);

    //bgColor = Qt::white;
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
    if(processed)
    {
        if(!data->selectionDefined())
            for(int i=0; i<allLevels.size(); i++)
                allLevels[i]->selected = 0;

        processSelection();
        repaint();
    }
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
    memLevel interconnect;
    interconnect.circleCenter = rect().center();

    for(int i=0; i<allLevels.size(); i++)
    {
        int depth = allLevels[i]->depth;

        if(depth < 2)
            continue;

        float midAngle = allLevels[i]->startAngle +
                         allLevels[i]->spanAngle/2;
        float val = allLevels[i]->numSelectedTransactions;
        float vmin = minTransactionsPerLevel[depth];
        float vmax = maxTransactionsPerLevel[depth];

        float thickness = scale(val,vmin,vmax,1,30);

        float minRad = allLevels[i]->radius - allLevels[i]->thickness*2 - 10;
        float maxRad = allLevels[i]->radius - allLevels[i]->thickness + 10;

        interconnect.constructPoly();

        QPointF startLine = rect().center() + polarToCartesian(QPointF(minRad,midAngle));
        QPointF endLine = rect().center() + polarToCartesian(QPointF(maxRad,midAngle));

        painter->setBrush(Qt::black);
        painter->setPen(QPen(Qt::black,thickness,Qt::SolidLine,Qt::FlatCap));

        painter->drawLine(startLine,endLine);
    }

    // Draw level segments
    for(int i=0; i<allLevels.size(); i++)
    {
        int depth = allLevels[i]->depth;

        if(depth == 0)
            continue;

        float val, vmin, vmax;
        if(depth == totalDepth-1)
        {
            val = allLevels[i]->numSelectedTransactions;
            vmin = minTransactionsPerLevel[depth];
            vmax = maxTransactionsPerLevel[depth];
        }
        else if (mode == COLORBY_CYCLES)
        {
            val = allLevels[i]->numSelectedCycles;
            vmin = minCyclesPerLevel[depth];
            vmax = maxCyclesPerLevel[depth];
        }
        else if(mode == COLORBY_SAMPLES)
        {
            val = allLevels[i]->numSelectedCycles;
            vmin = minCyclesPerLevel[depth];
            vmax = maxCyclesPerLevel[depth];
        }
        else
        {
            cerr << "PARAKEETS" << endl;
            return;
        }

        if(vmax-vmin == 0)
            vmax++;

        vmin *= minScale;

        painter->setBrush(valToColor(val,vmin,vmax,colorBarMin,colorBarMax));

        //if(depth == 1)
        //    painter->setBrush(QColor(94,60,153));
        //else if(depth == 2)
        //    painter->setBrush(QColor(178,171,210));
        //else if(depth == totalDepth-1)
        //    painter->setBrush(QColor(230,97,1));
        //else
        //    painter->setBrush(QColor(253,184,99));

        if(allLevels[i]->selected)
            painter->setPen(QPen(Qt::yellow,2));
        else
            painter->setPen(QPen(Qt::black,2));

        painter->drawPolygon(allLevels[i]->polygon.constData(),allLevels[i]->polygon.size());
    }

    // Draw id numbers
    for(int i=0; i<allLevels.size(); i++)
    {
        if(allLevels[i]->depth == 0)
            continue;

        QString label;
        painter->setPen(QPen(Qt::black));

        if(allLevels[i]->depth == totalDepth-1)
            label = "P" + QString::number(allLevels[i]->id);
        else if(allLevels[i]->depth > 1)
            label = "L" + QString::number(allLevels[i]->id);
        else if(allLevels[i]->depth == 1)
            label = "N" + QString::number(allLevels[i]->id);
        else
            label = "RAM";


        painter->save();
        painter->translate(allLevels[i]->midPoint);
        painter->rotate(90-(allLevels[i]->startAngle+allLevels[i]->spanAngle/2)/16);
        painter->drawText(-5,0,label);
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
    allLevels.clear();

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
    //if(processed)
    {
        lvlHeight += e->delta()/80;
        processTopoViz();
        repaint();
    }
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
    processTopoViz();
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
