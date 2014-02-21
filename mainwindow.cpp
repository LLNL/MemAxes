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
// Multiple selections, selection groups (classification)
// Drag pcoord selection
// VolumeViz selection changed

// NEW FEATURES
// Application domain view
// data addresses -> symbols (classification)
// Histogram/violin plot
// Filter/select by threshold/scatterplot

// APPLICATIONS
// XSBench w tanzima
// openmp scheduling policies

// FUTURE
// memory topology vis

// HARD-CODED
// CSV parsing knows what to expect
// VTK volume hard-coded to 16x16x16, ew
// source directory for codeviz

// CANT FIX
// "invalid drawable" on initialization of pcoords viz
// PostScript font performance note
// VTK not supported in mavericks
//      WARNING: Method userSpaceScaleFactor in class NSView is deprecated on 10.7 and later. It should not be used in new applications. Use convertRectToBacking: instead.
//      Crash on close

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("CorrelaViz"));

    /*
     * MainWindow
     */
    currDataObject = NULL;

    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(importData()));
    connect(ui->actionImport_Memory_Topology, SIGNAL(triggered()),this,SLOT(importMemTopo()));
    connect(ui->actionImport_Source, SIGNAL(triggered()),this,SLOT(selectSourceDir()));
    connect(ui->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(iterationChanged(int)));
    connect(ui->filterSelection, SIGNAL(clicked()), this, SLOT(doFilterSelection()));
    connect(ui->resetFilter, SIGNAL(clicked()), this, SLOT(doResetFilter()));
    connect(ui->resetSelection, SIGNAL(clicked()), this, SLOT(doResetSelection()));


    /*
     * Code Viz
     */

    codeViz = new CodeViz(this);
    ui->codeVizLayout->addWidget(codeViz);

    connect(ui->codeMetric, SIGNAL(currentIndexChanged(int)), codeViz, SLOT(setMetricDim(int)));
    connect(ui->lineNum, SIGNAL(currentIndexChanged(int)), codeViz, SLOT(setLineNumDim(int)));

    vizWidgets.push_back(codeViz);

    /*
     * Variable Viz
     */

    varViz = new VarViz(this);
    ui->varVizLayout->addWidget(varViz);

    connect(ui->codeMetric, SIGNAL(currentIndexChanged(int)), varViz, SLOT(setMetricDim(int)));
    connect(ui->varSel, SIGNAL(currentIndexChanged(int)), varViz, SLOT(setVarDim(int)));

    vizWidgets.push_back(varViz);

    /*
     * Code Editor
     */

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    codeEditor = new CodeEditor(this);
    codeEditor->setFont(font);
    codeEditor->setReadOnly(true);
    ui->codeEditorLayout->addWidget(codeEditor);

    connect(codeViz, SIGNAL(sourceFileSelected(QFile*)), codeEditor, SLOT(setFile(QFile*)));
    connect(codeViz, SIGNAL(sourceLineSelected(int)), codeEditor, SLOT(setLine(int)));
    connect(codeEditor, SIGNAL(lineSelected(int)), this, SLOT(selectLine(int)));


    /*
     * Volume Viz
     */

    /*
    volumeVizWidget = new VolumeVizWidget(this);
    ui->volVizLayout->addWidget(volumeVizWidget);

    connect(ui->minAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMinOpacity(int)));
    connect(ui->midAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMidOpacity(int)));
    connect(ui->maxAlpha, SIGNAL(valueChanged(int)), volumeVizWidget, SLOT(setMaxOpacity(int)));

    connect(ui->minVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMinVal(double)));
    connect(ui->midVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMidVal(double)));
    connect(ui->maxVal, SIGNAL(valueChanged(double)), volumeVizWidget, SLOT(setMaxVal(double)));

    connect(volumeVizWidget, SIGNAL(minValSet(double)), ui->minVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(midValSet(double)), ui->midVal, SLOT(setValue(double)));
    connect(volumeVizWidget, SIGNAL(maxValSet(double)), ui->maxVal, SLOT(setValue(double)));

    connect(ui->vol_xdim_box, SIGNAL(currentIndexChanged(int)), volumeVizWidget, SLOT(setXDim(int)));
    connect(ui->vol_ydim_box, SIGNAL(currentIndexChanged(int)), volumeVizWidget, SLOT(setYDim(int)));
    connect(ui->vol_zdim_box, SIGNAL(currentIndexChanged(int)), volumeVizWidget, SLOT(setZDim(int)));
    connect(ui->vol_wdim_box, SIGNAL(currentIndexChanged(int)), volumeVizWidget, SLOT(setWDim(int)));
    */

    /*
     * Memory Topology Viz
     */

    memViz = new MemTopoViz(this);
    ui->memoryLayout->addWidget(memViz);

    //connect(ui->dimSel,SIGNAL(currentIndexChanged(int)),selectionVizWidget,SLOT(setDim(int)));

    vizWidgets.push_back(memViz);

    /*
     * Selection Viz
     */

    SelectionVizWidget *selectionVizWidget = new SelectionVizWidget(this);
    ui->selectionLayout->addWidget(selectionVizWidget);

    connect(ui->dimSel,SIGNAL(currentIndexChanged(int)),selectionVizWidget,SLOT(setDim(int)));

    vizWidgets.push_back(selectionVizWidget);

    /*
     * Parallel Coords Viz
     */

    ParallelCoordinatesVizWidget *parallelCoordinatesViz = new ParallelCoordinatesVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    connect(ui->selOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setSelOpacity(int)));
    connect(ui->unselOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setUnselOpacity(int)));
    connect(ui->boxplotBox, SIGNAL(clicked(bool)), parallelCoordinatesViz, SLOT(setShowBoxPlots(bool)));
    connect(ui->histogramBox, SIGNAL(clicked(bool)), parallelCoordinatesViz, SLOT(setShowHistograms(bool)));

    vizWidgets.push_back(parallelCoordinatesViz);

    /*
     * All VizWidget Connections
     */

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
    //volumeVizWidget->selectionChangedSlot();
    //volumeVizWidget->update();
}

void MainWindow::visibilityChanged()
{
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->visibilityChangedSlot();
    }
    //volumeVizWidget->selectionChangedSlot();
    //volumeVizWidget->update();
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

    // Make new data
    dataObjects.push_back(dobj);
    currDataObject = dobj;

    // Set viz widget meta info
    ui->vol_xdim_box->clear();
    ui->vol_ydim_box->clear();
    ui->vol_zdim_box->clear();
    ui->vol_wdim_box->clear();
    ui->vol_tdim_box->clear();
    ui->dimSel->clear();
    ui->codeMetric->clear();
    ui->lineNum->clear();
    ui->varSel->clear();

    // Add dimension names
    ui->vol_xdim_box->addItems(currDataObject->meta);
    ui->vol_ydim_box->addItems(currDataObject->meta);
    ui->vol_zdim_box->addItems(currDataObject->meta);
    ui->vol_wdim_box->addItems(currDataObject->meta);
    ui->vol_tdim_box->addItems(currDataObject->meta);
    ui->dimSel->addItems(currDataObject->meta);
    ui->codeMetric->addItems(currDataObject->meta);
    ui->lineNum->addItems(currDataObject->meta);
    ui->varSel->addItems(currDataObject->meta);

    // Set default values
    ui->vol_xdim_box->setCurrentIndex(currDataObject->meta.indexOf("xidx"));
    ui->vol_ydim_box->setCurrentIndex(currDataObject->meta.indexOf("yidx"));
    ui->vol_zdim_box->setCurrentIndex(currDataObject->meta.indexOf("zidx"));
    ui->vol_wdim_box->setCurrentIndex(currDataObject->meta.indexOf("latency"));
    ui->vol_tdim_box->setCurrentIndex(currDataObject->meta.indexOf("iter"));
    ui->dimSel->setCurrentIndex(currDataObject->meta.indexOf("latency"));
    ui->codeMetric->setCurrentIndex(currDataObject->meta.indexOf("latency"));
    ui->lineNum->setCurrentIndex(currDataObject->meta.indexOf("line"));
    ui->varSel->setCurrentIndex(currDataObject->meta.indexOf("variable"));

    memViz->setEncDim(currDataObject->meta.indexOf("dataSource"));
    memViz->setLatDim(currDataObject->meta.indexOf("latency"));
    memViz->setCpuDim(currDataObject->meta.indexOf("cpu"));
    memViz->setNodeDim(currDataObject->meta.indexOf("node"));

    // Set viz widgets to use new data
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->setData(currDataObject);
        vizWidgets[i]->repaint();
    }
    //volumeVizWidget->setData(currDataObject);

    return 0;
}

int MainWindow::importMemTopo()
{
    // Open a file using a dialog
    QFileDialog dirDiag(this);
    QString topoFilename = dirDiag.getOpenFileName(this, tr("Select Source Directory"),
                                        "/Users/chai/Documents/Data/Hardware");

    memViz->loadHierarchyFromXML(topoFilename);

    return 0;
}

int MainWindow::selectSourceDir()
{
    // Open a file using a dialog
    QFileDialog dirDiag(this);
    dirDiag.setFileMode(QFileDialog::DirectoryOnly);
    sourceDir = dirDiag.getOpenFileName(this, tr("Select Source Directory"),
                                        "/Users/chai/Documents/Data");
    codeViz->setSourceDir(sourceDir);

    return 0;
}

void MainWindow::iterationChanged(int val)
{
    int dim = ui->vol_tdim_box->currentIndex();

    currDataObject->hideAll();
    currDataObject->filterByDimRange(dim,val-10,val+10);

    visibilityChanged();
}

void MainWindow::selectLine(int line)
{
    currDataObject->deselectAll();
    currDataObject->selectByDimRange(ui->lineNum->currentIndex(),line,line+1);
    selectionChanged();
}

void MainWindow::doFilterSelection()
{
    currDataObject->filterBySelection();
    visibilityChanged();
}

void MainWindow::doResetSelection()
{
    currDataObject->deselectAll();
    selectionChanged();
}

void MainWindow::doResetFilter()
{
    currDataObject->showAll();
    visibilityChanged();
}
