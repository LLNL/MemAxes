#include "vizwidget.h"

#include <QPaintEvent>

VizWidget::VizWidget()
{
    // GLWidget options
    setMinimumSize(200, 200);
    setAutoFillBackground(false);

    // Set painting variables
    backgroundColor = QBrush(QColor(204, 229, 255));
    selectColor = QBrush(Qt::yellow);
    vizProcessed = false;
}

void VizWidget::paintGL(QRect rect)
{
    // add custom gl calls here
}

void VizWidget::paintEvent(QPaintEvent *event)
{
    if(!vizProcessed)
        return;

    paintGL(event->rect());

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, event, 0);
    painter.end();
}

void VizWidget::resizeGL(int width, int height)
{
    winRect = QRect(0,0,width,height);
}

void VizWidget::processViz()
{

}

void VizWidget::selectionChangedSlot()
{
    repaint();
}

void VizWidget::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{

}
