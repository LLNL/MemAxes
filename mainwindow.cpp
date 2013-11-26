#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>

#include "parallelcoordinatesviz.h"
#include "volumevizwidget.h"

// BIG TODO LIST

// INTERFACE FIXES
// Reset parallel coordinates spacing
// Fix coordinate rearrangements
// Multiple selections, selection groups (classification)
// Drag pcoord selection

// NEW FEATURES
// Application domain view
// ip -> code domain (addr2line)
// data addresses -> symbols (classification)
// Histogram/violin plot
// Filter/select by threshold/scatterplot

// APPLICATIONS
// XSBench w tanzima
// openmp scheduling policies

// FUTURE
// memory topology vis

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("CorrelaViz"));

    currDataObject = NULL;

    // Create viz widgets
    volumeVizWidget = new VolumeVizWidget(this);
    ui->volVizLayout->addWidget(volumeVizWidget);

    SelectionVizWidget *selectionVizWidget = new SelectionVizWidget(this);
    ui->selectionLayout->addWidget(selectionVizWidget);

    connect(ui->dimSelector,SIGNAL(valueChanged(int)),selectionVizWidget,SLOT(setDim(int)));

    ParallelCoordinatesVizWidget *parallelCoordinatesViz = new ParallelCoordinatesVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    // Make UI connections
    // mainwindow
    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(importData()));
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(iterationChanged(int)));
    connect(ui->filterSelection, SIGNAL(clicked()), this, SLOT(doFilterSelection()));
    connect(ui->resetFilter, SIGNAL(clicked()), this, SLOT(doResetFilter()));

    connect(ui->minAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMinOpacity(int)));
    connect(ui->midAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMidOpacity(int)));
    connect(ui->maxAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMaxOpacity(int)));
    connect(ui->minVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMinVal(double)));
    connect(ui->midVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMidVal(double)));
    connect(ui->maxVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMaxVal(double)));
    connect(volumeVizWidget, SIGNAL(minValSet(double)), ui->minVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(midValSet(double)), ui->midVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(maxValSet(double)), ui->maxVal, SLOT(setValue(double)));

    connect(ui->vol_xdim, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setXDim(int)));
    connect(ui->vol_ydim, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setYDim(int)));
    connect(ui->vol_zdim, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setZDim(int)));
    connect(ui->vol_wdim, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setWDim(int)));

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
    volumeVizWidget->selectionChangedSlot();
    volumeVizWidget->update();
}

void MainWindow::visibilityChanged()
{
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->visibilityChangedSlot();
    }
    volumeVizWidget->selectionChangedSlot();
    volumeVizWidget->update();
}

int MainWindow::importData()
{
    // Open a file using a dialog
    QString dataFileName = QFileDialog::getOpenFileName(this, tr("Import Data"),
                                                     "/Users/chai/Documents/Data",
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

    currDataObject = dobj;

    return 0;
}

void MainWindow::iterationChanged(int val)
{
    int dim = ui->time_dim->value();

    currDataObject->hideAll();
    currDataObject->filterByDimRange(dim,val-10,val+10);

    visibilityChanged();
}

void MainWindow::doFilterSelection()
{
    currDataObject->filterBySelection();
    visibilityChanged();
}

void MainWindow::doResetFilter()
{
    currDataObject->showAll();
    visibilityChanged();
}
