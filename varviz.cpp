#include "varviz.h"

#include <QFile>
#include <QMouseEvent>
#include <iostream>
using namespace std;

bool operator<(const varBlock &lhs, const varBlock &rhs)
{
    return lhs.val > rhs.val; // reverse sort ;-)
}

VarViz::VarViz(QWidget *parent) :
    VizWidget(parent)
{
    margin = 10;
    metricDim = 0;
    varDim = 0;

    numVariableBlocks = 8;

    this->installEventFilter(this);
}

VarViz::~VarViz()
{
}

int VarViz::getVariableID(QString name)
{
    for(int i=0; i<varBlocks.size(); i++)
    {
        if(varBlocks[i].name == name)
            return i;
    }

    // First time we see this name, new entry
    varBlock newBlock = {name, 0, QRect()};
    varBlocks.push_back(newBlock);

    return varBlocks.size()-1;
}

void VarViz::processData()
{
    varMaxVal = 0;
    varBlocks.clear();

    // Get metric values
    int elem = 0;
    QVector<qreal>::Iterator p;
    for(elem=0, p=data->begin; p!=data->end; elem++, p+=data->numDimensions)
    {
        if(data->selectionDefined() && !data->selected(elem))
            continue;

        int varIdx = this->getVariableID(data->varNames[elem]);
        varBlocks[varIdx].val += *(p+metricDim);
        varMaxVal = fmax(varMaxVal,varBlocks[varIdx].val);
    }

    // Sort based on value
    qSort(varBlocks.begin(),varBlocks.end());

    processed = true;
}

void VarViz::selectionChangedSlot()
{
    processData();
    repaint();
}

void VarViz::drawQtPainter(QPainter *painter)
{
    drawSpace = QRect(this->rect().left()+margin,
                      this->rect().top()+margin,
                      width()-margin*2,
                      height()-margin*2);

    painter->fillRect(drawSpace, bgColor);

    if(!processed)
        return;

    int blockheight = 20;
    for(int i=0; i<numVariableBlocks && i<varBlocks.size(); i++)
    {
        varBlocks[i].block.setLeft(drawSpace.left());
        varBlocks[i].block.setTop(drawSpace.top()+i*blockheight);
        varBlocks[i].block.setWidth(varBlocks[i].val/varMaxVal*drawSpace.width());
        varBlocks[i].block.setHeight(blockheight);

        painter->fillRect(varBlocks[i].block,Qt::lightGray);
        painter->setPen(Qt::black);
        painter->drawText(varBlocks[i].block.topLeft()+QPoint(0,16),varBlocks[i].name);
    }
}

void VarViz::mouseReleaseEvent(QMouseEvent *e)
{
    for(int i=0; i<varBlocks.size(); i++)
    {
        QRect varSelectionBox(drawSpace.left(),
                              varBlocks[i].block.top(),
                              drawSpace.width(),
                              varBlocks[i].block.height());
        if(varSelectionBox.contains(e->pos()))
        {
            data->deselectAll();
            data->selectByVarName(varBlocks[i].name);

            emit variableSelected(i);
            emit selectionChangedSig();

            return;
        }
    }
}

void VarViz::setMetricDim(int indim)
{
    metricDim = indim;

    if(processed)
    {
        processData();
        update();
    }
}

void VarViz::setVarDim(int indim)
{
    varDim = indim;

    if(processed)
    {
        processData();
        update();
    }
}
