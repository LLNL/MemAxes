#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QVector>

#include "vizwidget.h"
#include "correlationmatrixviz.h"
#include "scatterplotviz.h"
#include "parallelcoordinatesviz.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void repaintAllVizWidgets();
    int importData();
    DataObject* parseCSVFile(QString dataFileName);

private:
    Ui::MainWindow *ui;

    QVector<VizWidget*> vizWidgets;
    QVector<DataObject*> dataObjects;
};

#endif // MAINWINDOW_H
