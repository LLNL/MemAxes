#include "ui_form.h"
#include "mainwindow.h"

#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>

#include "parallelcoordinatesviz.h"

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
    ui->menuBar->setNativeMenuBar(true);

    dataSet = new DataSetObject();

    con = new console(this);
    ui->consoleLayout->addWidget(con);

    QPlainTextEdit *console_input = new QPlainTextEdit();
    console_input->setFont(QFont("Consolas"));
    console_input->setMaximumHeight(28);
    ui->consoleLayout->addWidget(console_input);

    con->setConsoleInput(console_input);
    con->setDataSet(dataSet);

    /*
     * MainWindow
     */

    // File buttons
    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(loadData()));
    connect(ui->actionAdd_Dataset_2, SIGNAL(triggered()),this,SLOT(addData()));

    // Selection mode
    connect(ui->selectModeXOR, SIGNAL(toggled(bool)), this, SLOT(setSelectModeXOR(bool)));
    connect(ui->selectModeOR, SIGNAL(toggled(bool)), this, SLOT(setSelectModeOR(bool)));
    connect(ui->selectModeAND, SIGNAL(toggled(bool)), this, SLOT(setSelectModeAND(bool)));

    // Selection buttons
    connect(ui->selectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
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

    vizWidgets.push_back(codeViz);

    /*
     * Variable Viz
     */

    varViz = new VarViz(this);
    ui->varVizLayout->addWidget(varViz);

    vizWidgets.push_back(varViz);

    /*
     * Code Editor
     */

    codeEditor = new CodeEditor(this);
    codeEditor->setFont(QFont("Consolas"));
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

    connect(ui->memTopoColorByCycles,SIGNAL(toggled(bool)),memViz,SLOT(setColorByCycles(bool)));
    connect(ui->memTopoColorBySamples,SIGNAL(toggled(bool)),memViz,SLOT(setColorBySamples(bool)));
    connect(ui->memTopoVizModeIcicle,SIGNAL(toggled(bool)),memViz,SLOT(setVizModeIcicle(bool)));
    connect(ui->memTopoVizModeSunburst,SIGNAL(toggled(bool)),memViz,SLOT(setVizModeSunburst(bool)));

    vizWidgets.push_back(memViz);

    /*
     * Parallel Coords Viz
     */

    ParallelCoordinatesVizWidget *parallelCoordinatesViz = new ParallelCoordinatesVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    connect(ui->selOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setSelOpacity(int)));
    connect(ui->unselOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setUnselOpacity(int)));
    connect(ui->histogramBox, SIGNAL(clicked(bool)), parallelCoordinatesViz, SLOT(setShowHistograms(bool)));

    vizWidgets.push_back(parallelCoordinatesViz);

    /*
     * All VizWidgets
     */

    // Set viz widgets to use new data
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->setDataSet(dataSet);
        vizWidgets[i]->setConsole(con);
    }
    volumeVizWidget->setDataSet(dataSet);

    dataSet->setConsole(con);

    connect(con, SIGNAL(selectionChangedSig()), this, SLOT(selectionChangedSlot()));

    for(int i=0; i<vizWidgets.size(); i++)
    {
        connect(vizWidgets[i], SIGNAL(selectionChangedSig()), this, SLOT(selectionChangedSlot()));
        connect(this, SIGNAL(selectionChangedSig()), vizWidgets[i], SLOT(selectionChangedSlot()));
        connect(vizWidgets[i], SIGNAL(visibilityChangedSig()), this, SLOT(visibilityChangedSlot()));
        connect(this, SIGNAL(visibilityChangedSig()), vizWidgets[i], SLOT(visibilityChangedSlot()));
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::selectionChangedSlot()
{
    dataSet->selectionChanged();
    volumeVizWidget->selectionChangedSlot();
    emit selectionChangedSig();
}

void MainWindow::visibilityChangedSlot()
{
    dataSet->visibilityChanged();
    emit visibilityChangedSig();
}

int MainWindow::loadData()
{
    int err = 0;

    err = importHardwareTopology();
    if(err != 0)
        return err;

    err = selectSourceDirectory();
    if(err != 0)
        return err;

    err = addData();
    if(err != 0)
        return err;

    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->processData();
        vizWidgets[i]->update();
    }

    volumeVizWidget->processData();

    return 0;
}

int MainWindow::addData()
{
    int err = importData();

    if(err != 0)
        return err;

    visibilityChangedSlot();

    return 0;
}

int MainWindow::importData()
{
    QFileDialog dirDiag(this);
    QString dataFileName = dirDiag.getOpenFileName(this,
                                                   tr("Select Memory Access Samples File"),
                                                   "/Users/chai/Sources/case_studies/sample_data");
    if(dataFileName.isNull())
        return -1;

    dataSet->addData(dataFileName);

    return 0;
}

int MainWindow::importHardwareTopology()
{
    QFileDialog dirDiag(this);
    QString topoFilename = dirDiag.getOpenFileName(this,
                                                   tr("Select Memory Topology File"),
                                                   "/Users/chai/Sources/case_studies/hardware");
    if(topoFilename.isNull())
        return -1;

    dataSet->setHardwareTopology(topoFilename);

    return 0;
}

int MainWindow::selectSourceDirectory()
{
    sourceDir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select Source Directory"),
                                                  "/Users/chai/Sources/case_studies/code",
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
    if(sourceDir.isNull())
        return -1;

    codeViz->setSourceDir(sourceDir);
    con->append("Selected Source Directory : "+sourceDir);

    return 0;
}

void MainWindow::showSelectedOnly()
{
    dataSet->hideUnselected();
    visibilityChangedSlot();
}

void MainWindow::selectAllVisible()
{
    dataSet->selectAllVisible();
    selectionChangedSlot();
}

void MainWindow::selectAll()
{
    dataSet->selectAll();
    selectionChangedSlot();
}

void MainWindow::deselectAll()
{
    dataSet->deselectAll();
    selectionChangedSlot();
}

void MainWindow::showAll()
{
    dataSet->showAll();
    visibilityChangedSlot();
}

void MainWindow::hideSelected()
{
    dataSet->hideSelected();
    visibilityChangedSlot();
}

void MainWindow::setSelectModeAND(bool on)
{
    if(on)
        dataSet->setSelectionMode(MODE_FILTER);
}

void MainWindow::setSelectModeOR(bool on)
{
    if(on)
        dataSet->setSelectionMode(MODE_APPEND);
}

void MainWindow::setSelectModeXOR(bool on)
{
    if(on)
        dataSet->setSelectionMode(MODE_NEW);
}

void MainWindow::setCodeLabel(QFile *file)
{
    ui->codeLabel->setText(file->fileName());
}
