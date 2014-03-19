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
    void showSelectedOnly();
    void showAll();
    void hideSelected();
    void selectAllVisible();
    void deselectAll();
    void setSelectModeAND(bool on);
    void setSelectModeOR(bool on);
    void setSelectModeXOR(bool on);
    void setCodeLabel(QFile *file);

private:
    Ui::MainWindow *ui;
    CodeEditor *codeEditor;
    CodeViz *codeViz;
    MemTopoViz *memViz;
    SelectionVizWidget *selViz;
    VarViz *varViz;
    QString sourceDir;

    QVector<VizWidget*> vizWidgets;
    VolumeVizWidget *volumeVizWidget;

    DataObject *currDataObject;
    QVector<DataObject*> dataObjects;
};

#endif // MAINWINDOW_H
