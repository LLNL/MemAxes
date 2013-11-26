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

signals:
    void requestFilterByDim(int dim, double minv, double maxv);

public slots:
    void selectionChanged();
    void visibilityChanged();
    int importData();
    void iterationChanged(int val);
    void doFilterSelection();
    void doResetFilter();

private:
    Ui::MainWindow *ui;

    QVector<VizWidget*> vizWidgets;
    VolumeVizWidget *volumeVizWidget;

    DataObject *currDataObject;
    QVector<DataObject*> dataObjects;
};

#endif // MAINWINDOW_H
