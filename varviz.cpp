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
    margin = 0;
    numVariableBlocks = 8;

    this->setMinimumHeight(20);
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
    processed = false;

    if(dataSet->isEmpty())
        return;

    varMaxVal = 0;
    varBlocks.clear();

    // Get metric values
    int elem = 0;
    QVector<qreal>::Iterator p;
    for(int d=0; d<dataSet->size(); d++)
    {
        for(elem=0, p=dataSet->at(d)->begin; p!=dataSet->at(d)->end; elem++, p+=dataSet->at(d)->numDimensions)
        {
            if(dataSet->selectionDefined() && !dataSet->at(d)->selected(elem))
                continue;

            int varIdx = this->getVariableID(dataSet->at(d)->varNames[elem]);
            varBlocks[varIdx].val += *(p+dataSet->at(d)->latencyDim);
            varMaxVal = fmax(varMaxVal,varBlocks[varIdx].val);
        }
    }

    // Sort based on value
    qSort(varBlocks.begin(),varBlocks.end());

    processed = true;
}

void VarViz::selectionChangedSlot()
{
    if(processed)
    {
        processData();
        repaint();
    }
}

void VarViz::drawQtPainter(QPainter *painter)
{
    drawSpace = rect();

    painter->fillRect(rect(), bgColor);

    if(!processed)
        return;

    int numBlocks = min(numVariableBlocks, varBlocks.size());
    if(numBlocks == 0)
        return;

    int blockheight = drawSpace.height() / numBlocks;
    for(int i=0; i<numBlocks; i++)
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
            dataSet->selectByVarName(varBlocks[i].name);

            emit variableSelected(i);
            emit selectionChangedSig();

            return;
        }
    }
}
