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
#include "hwtopodrawable.h"

#include <QMouseEvent>
#include <QPair>
#include <QXmlStreamReader>
#include <QToolTip>

class HWTopoVizWidget : public VizWidget
{
    Q_OBJECT
public:
    HWTopoVizWidget(QWidget *parent = 0);

protected:
    void frameUpdate();
    void processData();
    void selectionChangedSlot();
    void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

signals:

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

public slots:
    void setColorByCycles(bool on) { if(on) { hwPainter->setDataMode(COLORBY_CYCLES); selectionChangedSlot(); } }
    void setColorBySamples(bool on) { if(on) { hwPainter->setDataMode(COLORBY_SAMPLES); selectionChangedSlot(); } }
    void setVizModeIcicle(bool on) { if(on) { hwPainter->setVizMode(ICICLE); selectionChangedSlot(); } }
    void setVizModeSunburst(bool on) { if(on) { hwPainter->setVizMode(SUNBURST); selectionChangedSlot(); } }

private:
    void selectSamplesWithinNode(HWNode *lvl);

private:
    HWTopoPainter *hwPainter;
    bool needsCalcMinMaxes;
};

#endif // MEMTOPOVIZ_H
