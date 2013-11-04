#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>

#include "parallelcoordinatesviz.h"
#include "volumevizwidget.h"

// for 11/6
// ****contour plots
// ****Metrics about your selection (% of total, avg lat, etc)
// ***** include temp/pressure/etc in different pcoords axes

// ***Reset parallel coordinates spacing
// ****Application domain view
// *ip -> code domain (addr2line)
// **data addresses -> symbols

// memory topology vis

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("CorrelaViz"));

    // Create viz widgets
    volumeVizWidget = new VolumeVizWidget(this);
    ui->volumeVizLayout->addWidget(volumeVizWidget);

    SelectionVizWidget *selectionVizWidget = new SelectionVizWidget(this);
    ui->selectionLayout->addWidget(selectionVizWidget);

    connect(ui->dimSelector,SIGNAL(valueChanged(int)),selectionVizWidget,SLOT(setDim(int)));

    ParallelCoordinatesVizWidget *parallelCoordinatesViz = new ParallelCoordinatesVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    // Make UI connections
    // mainwindow
    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(importData()));

    connect(ui->minAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMinOpacity(int)));
    connect(ui->midAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMidOpacity(int)));
    connect(ui->maxAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMaxOpacity(int)));
    connect(ui->minVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMinVal(double)));
    connect(ui->midVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMidVal(double)));
    connect(ui->maxVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMaxVal(double)));
    connect(volumeVizWidget, SIGNAL(minValSet(double)), ui->minVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(midValSet(double)), ui->midVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(maxValSet(double)), ui->maxVal, SLOT(setValue(double)));

    // pcoords
    connect(ui->selOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setSelOpacity(int)));
    connect(ui->unselOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setUnselOpacity(int)));

    // Add to viz widgets
    vizWidgets.push_back(parallelCoordinatesViz);
    vizWidgets.push_back(selectionVizWidget);

    for(int i=0; i<vizWidgets.size(); i++)
    {
        connect(vizWidgets[i], SIGNAL(selectionChangedSig()), this, SLOT(selectionChanged()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectionChanged()
{
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->selectionChangedSlot();
    }
    volumeVizWidget->processSelection();
    volumeVizWidget->update();
}

int MainWindow::importData()
{
    // Open a file using a dialog
    QString dataFileName = QFileDialog::getOpenFileName(this, tr("Import Data"),
                                                     "",
                                                     tr("Files (*.*)"));

    DataObject *dobj = new DataObject();
    int err = dobj->parseCSVFile(dataFileName);
    if(err != 0)
        return -1;

    dataObjects.push_back(dobj);

    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->setData(dobj);
        vizWidgets[i]->repaint();
    }
    volumeVizWidget->setData(dobj);

    return 0;
}
