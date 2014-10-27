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
#include "hardwaretopology.h"
#include "console.h"

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
    void selectionChangedSig();
    void visibilityChangedSig();

public slots:
    void selectionChangedSlot();
    void visibilityChangedSlot();
    int loadData();
    int addData();
    int importData();
    int importHardwareTopology();
    int selectSourceDirectory();
    void showSelectedOnly();
    void showAll();
    void hideSelected();
    void selectAllVisible();
    void selectAll();
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
    VarViz *varViz;
    QString sourceDir;

    QVector<VizWidget*> vizWidgets;
    VolumeVizWidget *volumeVizWidget;

    DataSetObject *dataSet;
    console *con;
};

#endif // MAINWINDOW_H
