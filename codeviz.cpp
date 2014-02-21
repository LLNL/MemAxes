#include "codeviz.h"

#include <QFile>
#include <QMouseEvent>
#include <iostream>
using namespace std;

bool operator<(const sourceBlock &lhs, const sourceBlock &rhs)
{
    return lhs.val > rhs.val; // reverse sort ;-)
}

bool operator<(const lineBlock &lhs, const lineBlock &rhs)
{
    return lhs.val > rhs.val; // reverse sort ;-)
}

CodeViz::CodeViz(QWidget *parent) :
    VizWidget(parent)
{
    margin = 10;
    metricDim = 0;
    lineNumDim = 0;

    numVisibleSourceBlocks = 2;
    numVisibleLineBlocks = 8;

    this->installEventFilter(this);

    // HARD CODE
    sourceDir = "/Users/chai/Documents/Data/XSBench";
}

CodeViz::~CodeViz()
{
    closeAll();
}

int CodeViz::getFileID(QString name)
{
    for(int i=0; i<sourceBlocks.size(); i++)
    {
        if(sourceBlocks[i].name == name)
            return i;
    }

    // First time we see this name, new entry
    QFile *src = new QFile(sourceDir+"/"+name);
    src->open(QIODevice::ReadOnly | QIODevice::Text);

    sourceBlock newBlock = {name, src, 0, QRect(), 0, QVector<lineBlock>()};
    sourceBlocks.push_back(newBlock);

    return sourceBlocks.size()-1;
}

int CodeViz::getLineID(sourceBlock *src, int line)
{
    for(int i=0; i<src->lineBlocks.size(); i++)
    {
        if(src->lineBlocks[i].line == line)
            return i;
    }

    // First time we see this line, new entry
    lineBlock newBlock = {line, 0, QRect()};
    src->lineBlocks.push_back(newBlock);

    return src->lineBlocks.size()-1;
}

void CodeViz::processData()
{
    closeAll();

    sourceMaxVal = 0;
    sourceBlocks.clear();

    // Get metric values
    int elem = 0;
    QVector<qreal>::Iterator p;
    for(elem=0, p=data->begin; p!=data->end; elem++, p+=data->numDimensions)
    {
        if(data->selectionDefined() && !data->selected(elem))
            continue;

        int sourceIdx = this->getFileID(data->fileNames[elem]);
        sourceBlocks[sourceIdx].val += *(p+metricDim);
        sourceMaxVal = fmax(sourceMaxVal,sourceBlocks[sourceIdx].val);

        int lineIdx = this->getLineID(&sourceBlocks[sourceIdx],*(p+lineNumDim));
        sourceBlocks[sourceIdx].lineBlocks[lineIdx].val += *(p+metricDim);

        sourceBlocks[sourceIdx].lineMaxVal = fmax(sourceBlocks[sourceIdx].lineMaxVal,
                                                  sourceBlocks[sourceIdx].lineBlocks[lineIdx].val);
    }

    // Sort based on value
    qSort(sourceBlocks.begin(),sourceBlocks.end());

    for(int j=0; j<numVisibleLineBlocks && j<sourceBlocks.size(); j++)
    {
        qSort(sourceBlocks[j].lineBlocks.begin(),sourceBlocks[j].lineBlocks.end());
    }

    emit sourceFileSelected(sourceBlocks[0].file);
    emit sourceLineSelected(sourceBlocks[0].lineBlocks[0].line);

    processed = true;
}

void CodeViz::selectionChangedSlot()
{
    processData();
    repaint();
}

void CodeViz::drawQtPainter(QPainter *painter)
{
    drawSpace = QRect(this->rect().left()+margin,
                      this->rect().top()+margin,
                      width()-margin*2,
                      height()-margin*2);

    painter->fillRect(drawSpace, bgColor);

    if(!processed)
        return;

    int blockheight = 20;
    for(int i=0; i<numVisibleSourceBlocks && i<sourceBlocks.size(); i++)
    {
        sourceBlocks[i].block.setLeft(drawSpace.left());
        sourceBlocks[i].block.setTop(drawSpace.top()+i*blockheight*numVisibleLineBlocks);
        sourceBlocks[i].block.setWidth(sourceBlocks[i].val/sourceMaxVal*drawSpace.width());
        sourceBlocks[i].block.setHeight(blockheight*numVisibleLineBlocks);

        painter->fillRect(sourceBlocks[i].block,Qt::lightGray);

        for(int j=0; j<numVisibleLineBlocks && j<sourceBlocks[i].lineBlocks.size(); j++)
        {
            sourceBlocks[i].lineBlocks[j].block.setLeft(drawSpace.left());
            sourceBlocks[i].lineBlocks[j].block.setTop(sourceBlocks[i].block.top()+blockheight*j);
            sourceBlocks[i].lineBlocks[j].block.setWidth(sourceBlocks[i].lineBlocks[j].val/sourceBlocks[i].lineMaxVal*sourceBlocks[i].block.width());
            sourceBlocks[i].lineBlocks[j].block.setHeight(blockheight);

            painter->fillRect(sourceBlocks[i].lineBlocks[j].block,Qt::gray);
            painter->setPen(Qt::white);
            painter->drawText(QPoint(sourceBlocks[i].block.right(),sourceBlocks[i].lineBlocks[j].block.top())
                              +QPoint(-40,16),
                              QString::number(sourceBlocks[i].lineBlocks[j].line));
        }

        painter->setPen(Qt::black);
        painter->drawText(sourceBlocks[i].block.topLeft()+QPoint(0,16),sourceBlocks[i].name);
    }
}

void CodeViz::mouseReleaseEvent(QMouseEvent *e)
{
    for(int i=0; i<sourceBlocks.size(); i++)
    {
        if(sourceBlocks[i].block.contains(e->pos()))
        {
            for(int j=0; j<sourceBlocks[i].lineBlocks.size(); j++)
            {
                QRect lineSelectionBox(sourceBlocks[i].block.left(),
                                       sourceBlocks[i].lineBlocks[j].block.top(),
                                       sourceBlocks[i].block.width(),
                                       sourceBlocks[i].lineBlocks[j].block.height());
                if(lineSelectionBox.contains(e->pos()))
                {
                    data->deselectAll();
                    data->selectByDimRange(lineNumDim,
                                           sourceBlocks[i].lineBlocks[j].line,
                                           sourceBlocks[i].lineBlocks[j].line+1);

                    emit sourceFileSelected(sourceBlocks[i].file);
                    emit sourceLineSelected(sourceBlocks[i].lineBlocks[j].line);
                    emit selectionChangedSig();

                    return;
                }
            }
        }
    }
}

void CodeViz::setMetricDim(int indim)
{
    metricDim = indim;

    if(processed)
    {
        processData();
        update();
    }
}

void CodeViz::setLineNumDim(int indim)
{
    lineNumDim = indim;

    if(processed)
    {
        processData();
        update();
    }
}

void CodeViz::setSourceDir(QString dir)
{
    sourceDir = dir;
}

void CodeViz::closeAll()
{
    for(int i=0; i<sourceBlocks.size(); i++)
    {
        sourceBlocks[i].file->close();
    }
}
