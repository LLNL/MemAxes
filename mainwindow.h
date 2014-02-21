#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QVector>

#include "vizwidget.h"
#include "volumevizwidget.h"
#include "selectionviz.h"
#include "codeviz.h"
#include "varviz.h"
#include "memtopoviz.h"
#include "codeeditor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void requestFilterByDim(int dim, double minv, double maxv);

public slots:
    void selectionChanged();
    void visibilityChanged();
    int importData();
    int importMemTopo();
    int selectSourceDir();
    void iterationChanged(int val);
    void selectLine(int line);
    void doFilterSelection();
    void doResetFilter();
    void doResetSelection();

private:
    Ui::MainWindow *ui;
    CodeEditor *codeEditor;
    CodeViz *codeViz;
    MemTopoViz *memViz;
    VarViz *varViz;
    QString sourceDir;

    QVector<VizWidget*> vizWidgets;
    VolumeVizWidget *volumeVizWidget;

    DataObject *currDataObject;
    QVector<DataObject*> dataObjects;
};

#endif // MAINWINDOW_H
