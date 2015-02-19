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

#include "vizwidget.h"

#include <iostream>
using namespace std;

#include <QPaintEvent>
#include <QElapsedTimer>

VizWidget::VizWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    setAutoFillBackground(false);
    setMinimumSize(200, 200);
    setWindowTitle(tr("Viz"));

    margin = 20;
    bgColor = QColor(248,248,255);
    processed = false;
    needsRepaint = false;

    dataSet = NULL;
}

VizWidget::~VizWidget()
{
}

QSize VizWidget::sizeHint() const
{
    return QSize(400, 400);
}

void VizWidget::frameUpdate()
{
   if(needsRepaint)
   {
       repaint();
       needsRepaint = false;
   }
}

void VizWidget::selectionChangedSlot()
{
    needsRepaint = true;
}

void VizWidget::visibilityChangedSlot()
{
    needsRepaint = true;
}

void VizWidget::initializeGL()
{
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH);
}

void VizWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QElapsedTimer frameTimer;
    qint64 frameElapsed;

    frameTimer.start();

    // Clear
    makeCurrent();
    qglClearColor(bgColor);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw native GL
    beginNativeGL();
    {
        paintGL();
    }
    endNativeGL();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawQtPainter(&painter);
    painter.end();

    // show fps
    if(0)
    {
        frameElapsed = frameTimer.nsecsElapsed();
        double seconds = (double)frameElapsed * 1e-9;
        double fps = 1.0 / seconds;

        QPainter fpspainter(this);
        fpspainter.setRenderHint(QPainter::Antialiasing);
        fpspainter.drawText(rect().topRight()+QPoint(-50,10),
                            QString::number(fps));
        fpspainter.end();
    }
}

void VizWidget::setDataSet(DataObject *iDataSet)
{
    dataSet = iDataSet;
}

void VizWidget::setConsole(console *iCon)
{
    con = iCon;
}

void VizWidget::processData()
{
}

void VizWidget::paintGL()
{
}

void VizWidget::drawNativeGL()
{
}

void VizWidget::drawQtPainter(QPainter *painter)
{
    Q_UNUSED(painter);
}

void VizWidget::beginNativeGL()
{
    makeCurrent();

    // Qt 5.X BUG
    //int width2x = width()*2;
    //int height2x = height()*2;
    //glViewport(0, 0, width2x, height2x);

    glViewport(0, 0, width(), height());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
}

void VizWidget::endNativeGL()
{
    makeCurrent();

    // Revert settings for painter
    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}
