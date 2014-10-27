#ifndef CODEVIZ_H
#define CODEVIZ_H

#include "vizwidget.h"

struct lineBlock
{
    int line;
    qreal val;
    QRect block;
};

struct sourceBlock
{
    QString name;
    QFile *file;
    qreal val;
    QRect block;

    qreal lineMaxVal;
    QVector<lineBlock> lineBlocks;
};

class CodeViz : public VizWidget
{
    Q_OBJECT
public:
    CodeViz(QWidget *parent = 0);
    ~CodeViz();

signals:
    void sourceFileSelected(QFile *file);
    void sourceLineSelected(int line);

protected:
    void processData();
    void selectionChangedSlot();
    //void visibilityChangedSlot();
    void drawQtPainter(QPainter *painter);

    void mouseReleaseEvent(QMouseEvent *e);

public slots:
    void setSourceDir(QString dir);

private:
    int getFileID(QString name);
    int getLineID(sourceBlock *src, int line);
    void closeAll();

private:
    int margin;
    QRect drawSpace;

    QString sourceDir;

    int numVisibleSourceBlocks;
    int numVisibleLineBlocks;

    qreal sourceMaxVal;
    QVector<sourceBlock> sourceBlocks;
};

#endif // CODEVIZ_H
