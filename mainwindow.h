#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QVector>

#include "vizwidget.h"
#include "volumevizwidget.h"
#include "selectionviz.h"

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
    void selectionChanged();
    int importData();

private:
    Ui::MainWindow *ui;

    QVector<VizWidget*> vizWidgets;
    VolumeVizWidget *volumeVizWidget;

    QVector<DataObject*> dataObjects;
};

#endif // MAINWINDOW_H
