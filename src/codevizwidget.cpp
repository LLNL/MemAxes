//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#include "codevizwidget.h"

#include <QFile>
#include <QMouseEvent>

#include <iostream>
#include <algorithm>

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
    margin = 0;

    numVisibleSourceBlocks = 2;
    numVisibleLineBlocks = 8;

    this->setMinimumHeight(20);
    this->installEventFilter(this);

    processed = false;
    sourceDir = "NOT SELECTED";
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
    QString srcFile = sourceDir+"/"+name;
    QFile *src = new QFile(srcFile);
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
    processed = false;

    closeAll();

    sourceMaxVal = 0;
    sourceBlocks.clear();

    // Get metric values
    int elem = 0;
    QVector<qreal>::Iterator p;
    for(elem=0, p=dataSet->begin; p!=dataSet->end; elem++, p+=dataSet->numDimensions)
    {
        if(dataSet->selectionDefined() && !dataSet->selected(elem))
            continue;

        int sourceIdx = this->getFileID(dataSet->fileNames[elem]);
        sourceBlocks[sourceIdx].val += *(p+dataSet->latencyDim);
        sourceMaxVal = std::max(sourceMaxVal,sourceBlocks[sourceIdx].val);

        int lineIdx = this->getLineID(&sourceBlocks[sourceIdx],*(p+dataSet->lineDim));
        sourceBlocks[sourceIdx].lineBlocks[lineIdx].val += *(p+dataSet->latencyDim);

        sourceBlocks[sourceIdx].lineMaxVal = std::max(sourceBlocks[sourceIdx].lineMaxVal,
                                                  sourceBlocks[sourceIdx].lineBlocks[lineIdx].val);
    }

    if(sourceBlocks.empty())
    {
        processed = true;
        return;
    }

    // Sort based on value
    qSort(sourceBlocks.begin(),sourceBlocks.end());

    for(int j=0; j<sourceBlocks.size(); j++)
        qSort(sourceBlocks[j].lineBlocks.begin(),sourceBlocks[j].lineBlocks.end());

    emit sourceFileSelected(sourceBlocks[0].file);
    emit sourceLineSelected(sourceBlocks[0].lineBlocks[0].line);

    processed = true;
}

void CodeViz::selectionChangedSlot()
{
    if(processed)
    {
        processData();
        needsRepaint = true;
    }
}

void CodeViz::drawQtPainter(QPainter *painter)
{
    drawSpace = rect();

    painter->fillRect(drawSpace, bgColor);

    if(!processed || sourceBlocks.empty())
        return;

    int numBlocks = std::min(numVisibleSourceBlocks,sourceBlocks.size());
    int blockHeight = drawSpace.height() / numBlocks;
    for(int i=0; i<numBlocks; i++)
    {
        sourceBlocks[i].block.setLeft(drawSpace.left());
        sourceBlocks[i].block.setTop(drawSpace.top()+i*blockHeight);
        sourceBlocks[i].block.setWidth(sourceBlocks[i].val/sourceMaxVal*drawSpace.width());
        sourceBlocks[i].block.setHeight(blockHeight);

        painter->fillRect(sourceBlocks[i].block,Qt::lightGray);

        int numLines = std::min(numVisibleLineBlocks,sourceBlocks[i].lineBlocks.size());
        int lineHeight = blockHeight / numLines;
        for(int j=0; j<numLines; j++)
        {
            sourceBlocks[i].lineBlocks[j].block.setLeft(drawSpace.left());
            sourceBlocks[i].lineBlocks[j].block.setTop(sourceBlocks[i].block.top()+j*lineHeight);
            sourceBlocks[i].lineBlocks[j].block.setWidth(sourceBlocks[i].lineBlocks[j].val/sourceBlocks[i].lineMaxVal*sourceBlocks[i].block.width());
            sourceBlocks[i].lineBlocks[j].block.setHeight(lineHeight);

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
        QRect sourceSelectionBox(sourceBlocks[i].block.left(),
                                 sourceBlocks[i].block.top(),
                                 rect().width(),
                                 sourceBlocks[i].block.height());
        if(sourceSelectionBox.contains(e->pos()))
        {
            for(int j=0; j<sourceBlocks[i].lineBlocks.size(); j++)
            {
                QRect lineSelectionBox(sourceBlocks[i].block.left(),
                                       sourceBlocks[i].lineBlocks[j].block.top(),
                                       rect().width(),
                                       sourceBlocks[i].lineBlocks[j].block.height());
                if(lineSelectionBox.contains(e->pos()))
                {
                    int dim = dataSet->lineDim;
                    qreal lineval = sourceBlocks[i].lineBlocks[j].line;

                    ElemSet es = dataSet->createDimRangeQuery(dim,lineval-1,lineval);
                    dataSet->selectSet(es);

                    emit sourceFileSelected(sourceBlocks[i].file);
                    emit sourceLineSelected(sourceBlocks[i].lineBlocks[j].line);
                    emit selectionChangedSig();

                    return;
                }
            }
        }
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
