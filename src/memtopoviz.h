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
#ifndef MEMTOPOVIZ_H
#define MEMTOPOVIZ_H

#include "vizwidget.h"

#include <QMouseEvent>
#include <QPair>
#include <QXmlStreamReader>
#include <QToolTip>

enum VizMode
{
    ICICLE = 0,
    SUNBURST
};

enum DataMode
{
    COLORBY_SAMPLES = 0,
    COLORBY_CYCLES
};

typedef QPair<hardwareResourceNode*,QRectF> NodeBox;
typedef QPair<QColor,QRectF> ColoredRect;

class MemTopoViz : public VizWidget
{
    Q_OBJECT
public:
    MemTopoViz(QWidget *parent = 0);

protected:
    void processData();
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

signals:

public slots:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

    void setColorByCycles(bool on) { if(on) { dataMode = COLORBY_CYCLES; selectionChangedSlot(); } }
    void setColorBySamples(bool on) { if(on) { dataMode = COLORBY_SAMPLES; selectionChangedSlot(); } }
    void setVizModeIcicle(bool on) { if(on) { vizMode = ICICLE; selectionChangedSlot(); } }
    void setVizModeSunburst(bool on) { if(on) { vizMode = SUNBURST; selectionChangedSlot(); } }

private:
    void calcMinMaxes();
    void resizeNodeBoxes();
    void zoomVertical(hardwareResourceNode *node, int dir);
    void zoomHorizontal(hardwareResourceNode *node, int dir);
    hardwareResourceNode* nodeAtPosition(QPoint p);
    void selectSamplesWithinNode(hardwareResourceNode *lvl);

private:

    QVector<NodeBox> nodeBoxes;
    QVector<ColoredRect> nodeDataBoxes;
    QVector<RealRange> depthValRanges;

    DataMode dataMode;
    VizMode vizMode;
    ColorMap colorMap;

    QVector<qreal> minCyclesPerLevel;
    QVector<qreal> maxCyclesPerLevel;

    QVector<qreal> minTransactionsPerLevel;
    QVector<qreal> maxTransactionsPerLevel;

    QVector<qreal> minSamplesPerLevel;
    QVector<qreal> maxSamplesPerLevel;

    IntRange depthRange;
    QVector<IntRange> widthRange;
};

#endif // MEMTOPOVIZ_H
