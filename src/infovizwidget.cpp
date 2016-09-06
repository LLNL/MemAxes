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

#include "infovizwidget.h"

#include <QFile>
#include <QMouseEvent>

#include <iostream>
#include <algorithm>

#include <math.h>

bool operator<(const infoBlock &lhs, const infoBlock &rhs)
{
    return lhs.val > rhs.val; // reverse sort ;-)
}

InfoViz::InfoViz(QWidget *parent) :
    VizWidget(parent)
{
    margin = 0;
    numVariableBlocks = 20;
    variable = QString("");

    this->setMinimumHeight(20);
    this->installEventFilter(this);
}

InfoViz::~InfoViz()
{
}

int InfoViz::getVariableID(QString name)
{
    for(int i=0; i<varBlocks.size(); i++)
    {
        if(varBlocks[i].name == name)
            return i;
    }

    // First time we see this name, new entry
    infoBlock newBlock = {name, 0, QRect()};
    varBlocks.push_back(newBlock);

    return varBlocks.size()-1;
}

void InfoViz::processData()
{
    processed = false;

    varMaxVal = 0;
    varBlocks.clear();

    // Get metric values
    int elem = 0;
    QVector<qreal>::Iterator p;
    for(elem=0, p=dataSet->begin; p!=dataSet->end; elem++, p+=dataSet->numDimensions)
    {
        if(dataSet->selectionDefined() && !dataSet->selected(elem))
            continue;

        if(dataSet->infovals[variable][elem].isEmpty())
            continue;

        int varIdx = this->getVariableID(dataSet->infovals[variable][elem]);
        varBlocks[varIdx].val += *(p+dataSet->latencyDim);
        varMaxVal = std::max(varMaxVal,varBlocks[varIdx].val);
    }

    // Sort based on value
    qSort(varBlocks.begin(),varBlocks.end());

    processed = true;
}

void InfoViz::selectionChangedSlot()
{
    if(processed)
    {
        processData();
        repaint();
    }
}

void InfoViz::drawQtPainter(QPainter *painter)
{
    drawSpace = rect();

    painter->fillRect(rect(), bgColor);

    if(!processed)
        return;

    int numBlocks = std::min(numVariableBlocks, varBlocks.size());
    if(numBlocks == 0)
        return;

    int blockwidth = drawSpace.width() / numBlocks;
    for(int i=0; i<numBlocks; i++)
    {
        varBlocks[i].block.setLeft(drawSpace.left()+i*blockwidth);
        varBlocks[i].block.setWidth(blockwidth);
        varBlocks[i].block.setTop(drawSpace.height() - varBlocks[i].val/varMaxVal*drawSpace.height());
        varBlocks[i].block.setBottom(drawSpace.bottom());

        painter->fillRect(varBlocks[i].block,Qt::lightGray);
    }

    for(int i=0; i<numBlocks; i++)
    {
        painter->save();
        painter->translate(varBlocks[i].block.bottomRight() - QPoint(10, 10));
        painter->rotate(-90);
        painter->setPen(Qt::black);
        painter->drawText(0, 0, varBlocks[i].name);
        painter->restore();
    }
}

void InfoViz::mouseReleaseEvent(QMouseEvent *e)
{
    for(int i=0; i<varBlocks.size(); i++)
    {
        QRect varSelectionBox(varBlocks[i].block.left(),
                              drawSpace.top(),
                              varBlocks[i].block.width(),
                              drawSpace.height());
        if(varSelectionBox.contains(e->pos()))
        {
            ElemSet es = dataSet->createStringQuery(variable, varBlocks[i].name);
            dataSet->selectSet(es);

            emit variableSelected(i);
            emit selectionChangedSig();

            return;
        }
    }
}
