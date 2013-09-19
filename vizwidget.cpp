#include "vizwidget.h"

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

void VizWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, event, 0);
    painter.end();
}

void VizWidget::processViz()
{

}

void VizWidget::paint(QPainter *painter, QPaintEvent *event, int elapsed)
{

}
