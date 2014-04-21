#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>

#include "parallelcoordinatesviz.h"
#include "volumevizwidget.h"

// BIG TODO LIST

// BUGS
// Check HARD CODE!
// mem topo selection

// NEW FEATURES
// Mem topo 1d memory range
// Multiple selections, selection groups (classification)

// APPLICATIONS
// LibNUMA (move_pages(x,x,NULL,...)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("MemAxes"));

    /*
     * MainWindow
     */
    currDataObject = NULL;

    // File buttons
    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(loadData()));

    // Selection mode
    connect(ui->selectModeXOR, SIGNAL(toggled(bool)), this, SLOT(setSelectModeXOR(bool)));
    connect(ui->selectModeOR, SIGNAL(toggled(bool)), this, SLOT(setSelectModeOR(bool)));
    connect(ui->selectModeAND, SIGNAL(toggled(bool)), this, SLOT(setSelectModeAND(bool)));

    // Selection buttons
    connect(ui->deselectAll, SIGNAL(clicked()), this, SLOT(deselectAll()));
    connect(ui->selectAllVisible, SIGNAL(clicked()), this, SLOT(selectAllVisible()));

    // Visibility buttons
    connect(ui->hideSelected, SIGNAL(clicked()), this, SLOT(hideSelected()));
    connect(ui->showSelectedOnly, SIGNAL(clicked()), this, SLOT(showSelectedOnly()));
    connect(ui->showAll, SIGNAL(clicked()), this, SLOT(showAll()));

    /*
     * Code Viz
     */

    codeViz = new CodeViz(this);
    ui->codeVizLayout->addWidget(codeViz);

    //connect(codeViz, SIGNAL(sourceFileSelected(QFile*)), this, SLOT(setCodeLabel(QString)));
    //connect(ui->codeMetric, SIGNAL(currentIndexChanged(int)), codeViz, SLOT(setMetricDim(int)));
    //connect(ui->lineNum, SIGNAL(currentIndexChanged(int)), codeViz, SLOT(setLineNumDim(int)));

    vizWidgets.push_back(codeViz);

    /*
     * Variable Viz
     */

    varViz = new VarViz(this);
    ui->varVizLayout->addWidget(varViz);

    //connect(ui->codeMetric, SIGNAL(currentIndexChanged(int)), varViz, SLOT(setMetricDim(int)));
    //connect(ui->varSel, SIGNAL(currentIndexChanged(int)), varViz, SLOT(setVarDim(int)));

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

    connect(codeViz, SIGNAL(sourceFileSelected(QFile*)), this, SLOT(setCodeLabel(QFile*)));
    connect(codeViz, SIGNAL(sourceFileSelected(QFile*)), codeEditor, SLOT(setFile(QFile*)));
    connect(codeViz, SIGNAL(sourceLineSelected(int)), codeEditor, SLOT(setLine(int)));


    /*
     * Volume Viz
     */

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

    /*
     * Memory Topology Viz
     */

    memViz = new MemTopoViz(this);
    ui->memoryLayout->addWidget(memViz);

    connect(ui->minScale, SIGNAL(sliderMoved(int)), memViz, SLOT(setMinScale(int)));
    connect(ui->memTopoColorByCycles,SIGNAL(toggled(bool)),memViz,SLOT(setColorByCycles(bool)));
    connect(ui->memTopoColorBySamples,SIGNAL(toggled(bool)),memViz,SLOT(setColorBySamples(bool)));

    vizWidgets.push_back(memViz);

    /*
     * Selection Viz
     */

    selViz = new SelectionVizWidget(this);
    ui->selectionLayout->addWidget(selViz);

    connect(ui->selectionChartWeightByCycles,SIGNAL(toggled(bool)),selViz,SLOT(setWeightModeLatency(bool)));
    connect(ui->selectionChartWeightBySamples,SIGNAL(toggled(bool)),selViz,SLOT(setWeightModeSamples(bool)));

    vizWidgets.push_back(selViz);

    /*
     * Parallel Coords Viz
     */

    ParallelCoordinatesVizWidget *parallelCoordinatesViz = new ParallelCoordinatesVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    connect(ui->selOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setSelOpacity(int)));
    connect(ui->unselOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setUnselOpacity(int)));
    //connect(ui->boxplotBox, SIGNAL(clicked(bool)), parallelCoordinatesViz, SLOT(setShowBoxPlots(bool)));
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
        vizWidgets[i]->selectionChangedSlot();
    volumeVizWidget->selectionChangedSlot();
}

void MainWindow::visibilityChanged()
{
    for(int i=0; i<vizWidgets.size(); i++)
        vizWidgets[i]->visibilityChangedSlot();
    volumeVizWidget->selectionChangedSlot();
}

int MainWindow::loadData()
{
    int err = 0;

    err = importData();

    if(err != 0)
        return err;

    selectSourceDir();
    if(err != 0)
        return err;

    importMemTopo();
    if(err != 0)
        return err;

    return 0;
}

int MainWindow::importData()
{
    QString dataFileName = QFileDialog::getOpenFileName(this, tr("Import Data"));
    if(dataFileName.isNull())
        return -1;

    DataObject *dobj = new DataObject();
    int err = dobj->parseCSVFile(dataFileName);
    if(err != 0)
        return -1;

    // Make new data
    dataObjects.push_back(dobj);
    currDataObject = dobj;

    // Set default values
    volumeVizWidget->setMapDim(currDataObject->meta.indexOf("map3D"));
    volumeVizWidget->setXDim(currDataObject->meta.indexOf("xidx"));
    volumeVizWidget->setYDim(currDataObject->meta.indexOf("yidx"));
    volumeVizWidget->setZDim(currDataObject->meta.indexOf("zidx"));
    volumeVizWidget->setWDim(currDataObject->meta.indexOf("latency"));

    memViz->setEncDim(currDataObject->meta.indexOf("dataSource"));
    memViz->setLatDim(currDataObject->meta.indexOf("latency"));
    memViz->setCpuDim(currDataObject->meta.indexOf("cpu"));
    memViz->setNodeDim(currDataObject->meta.indexOf("node"));

    varViz->setMetricDim(currDataObject->meta.indexOf("latency"));
    varViz->setVarDim(currDataObject->meta.indexOf("variable"));

    codeViz->setLineNumDim(currDataObject->meta.indexOf("line"));
    codeViz->setMetricDim(currDataObject->meta.indexOf("latency"));


    // Set viz widgets to use new data
    for(int i=0; i<vizWidgets.size(); i++)
        vizWidgets[i]->setData(currDataObject);
    volumeVizWidget->setData(currDataObject);

    return 0;
}

int MainWindow::importMemTopo()
{
    // Open a file using a dialog
    QFileDialog dirDiag(this);

    QString initDir("/Users/chai/Documents/Data/Hardware");
    QString topoFilename = dirDiag.getOpenFileName(this, tr("Select Memory Topology File"),
                                                   ""//initDir
                                                   );
    if(topoFilename.isNull())
        return -1;

    memViz->loadHierarchyFromXML(topoFilename);

    if(currDataObject)
        memViz->setData(currDataObject);

    return 0;
}

int MainWindow::selectSourceDir()
{
    QString initDir("/Users/chai/Sources/case_studies");
    sourceDir = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"),
                                                  "", // initDir
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
    if(sourceDir.isNull())
        return -1;

    codeViz->setSourceDir(sourceDir);
    selectionChanged();

    return 0;
}

void MainWindow::showSelectedOnly()
{
    currDataObject->hideUnselected();
    visibilityChanged();
}

void MainWindow::selectAllVisible()
{
    currDataObject->selectAllVisible();
    selectionChanged();
}

void MainWindow::deselectAll()
{
    currDataObject->deselectAll();
    selectionChanged();
}

void MainWindow::showAll()
{
    currDataObject->showAll();
    visibilityChanged();
}

void MainWindow::hideSelected()
{
    currDataObject->hideSelected();
    visibilityChanged();
}

void MainWindow::setSelectModeAND(bool on)
{
    if(on)
        currDataObject->setSelectionModeAND();
}

void MainWindow::setSelectModeOR(bool on)
{
    if(on)
        currDataObject->setSelectionModeOR();
}

void MainWindow::setSelectModeXOR(bool on)
{
    if(on)
        currDataObject->setSelectionModeXOR();
}

void MainWindow::setCodeLabel(QFile *file)
{
    ui->codeLabel->setText(file->fileName());
}
