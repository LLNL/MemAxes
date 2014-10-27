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
}

VizWidget::~VizWidget()
{
}

QSize VizWidget::sizeHint() const
{
    return QSize(400, 400);
}

void VizWidget::selectionChangedSlot()
{
    repaint();
}

void VizWidget::visibilityChangedSlot()
{
    repaint();
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

void VizWidget::setDataSet(DataSetObject *iDataSet)
{
    dataSet = iDataSet;
    processData();
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
