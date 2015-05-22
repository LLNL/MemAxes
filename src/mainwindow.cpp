//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#include "ui_form.h"
#include "mainwindow.h"

#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("MemAxes"));
    ui->menuBar->setNativeMenuBar(true);

    dataSet = new DataObject();

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
     * Memory Topology Viz
     */

    memViz = new HWTopoVizWidget(this);
    ui->memoryLayout->addWidget(memViz);

    connect(ui->memTopoColorByCycles,SIGNAL(toggled(bool)),memViz,SLOT(setColorByCycles(bool)));
    connect(ui->memTopoColorBySamples,SIGNAL(toggled(bool)),memViz,SLOT(setColorBySamples(bool)));
    connect(ui->memTopoVizModeIcicle,SIGNAL(toggled(bool)),memViz,SLOT(setVizModeIcicle(bool)));
    connect(ui->memTopoVizModeSunburst,SIGNAL(toggled(bool)),memViz,SLOT(setVizModeSunburst(bool)));

    vizWidgets.push_back(memViz);

    /*
     * Parallel Coords Viz
     */

    PCVizWidget *parallelCoordinatesViz = new PCVizWidget(this);
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    vizWidgets.push_back(parallelCoordinatesViz);

    /*
     * Axis Viz
     */

    AxisVizWidget *axisViz = new AxisVizWidget(this);
    ui->singleAxisLayout->addWidget(axisViz);

    connect(ui->selectAxis, SIGNAL(valueChanged(int)), axisViz, SLOT(setDimension(int)));
    connect(ui->setClusterDepth, SIGNAL(valueChanged(int)), axisViz, SLOT(setClusterDepth(int)));
    connect(ui->setNumBins, SIGNAL(valueChanged(int)), axisViz, SLOT(setNumBins(int)));
    connect(ui->axisDrawHists, SIGNAL(stateChanged(int)), axisViz, SLOT(setDrawHists(int)));
    connect(ui->axisDrawClusters, SIGNAL(stateChanged(int)), axisViz, SLOT(setDrawClusters(int)));
    connect(ui->axisDrawMetrics, SIGNAL(stateChanged(int)), axisViz, SLOT(setDrawMetrics(int)));
    connect(ui->metricBox, SIGNAL(currentIndexChanged(int)), axisViz, SLOT(setMetric(int)));
    connect(ui->calcMetrics, SIGNAL(clicked()), axisViz, SLOT(setCalcMetrics()));
    connect(ui->calcClusters, SIGNAL(clicked()), axisViz, SLOT(setCalcClusters()));

    vizWidgets.push_back(axisViz);

    /*
     * All VizWidgets
     */

    // Set viz widgets to use new data
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->setDataSet(dataSet);
    }

    dataSet->setConsole(con);

    connect(con, SIGNAL(selectionChangedSig()), this, SLOT(selectionChangedSlot()));

    for(int i=0; i<vizWidgets.size(); i++)
    {
        connect(vizWidgets[i], SIGNAL(selectionChangedSig()), this, SLOT(selectionChangedSlot()));
        connect(this, SIGNAL(selectionChangedSig()), vizWidgets[i], SLOT(selectionChangedSlot()));
        connect(vizWidgets[i], SIGNAL(visibilityChangedSig()), this, SLOT(visibilityChangedSlot()));
        connect(this, SIGNAL(visibilityChangedSig()), vizWidgets[i], SLOT(visibilityChangedSlot()));
    }

    frameTimer = new QTimer(this);
    frameTimer->setInterval(1000/60); // 60fps
    connect(frameTimer,SIGNAL(timeout()),this,SLOT(frameUpdateAll()));
    frameTimer->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::frameUpdateAll()
{
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->frameUpdate();
    }
}

void MainWindow::selectionChangedSlot()
{
    dataSet->selectionChanged();
    emit selectionChangedSig();
}

void MainWindow::visibilityChangedSlot()
{
    dataSet->visibilityChanged();
    emit visibilityChangedSig();
}

void errdiag(QString str)
{
    QErrorMessage errmsg;
    errmsg.showMessage(str);
    errmsg.exec();
}

int MainWindow::loadData()
{
    int err = 0;

    err = selectDataDirectory();
    if(err != 0)
        return err;

    QString sourceDir(dataDir+QString("/src/"));
    codeViz->setSourceDir(sourceDir);

    QString topoDir(dataDir+QString("/hardware.xml"));
    err = dataSet->loadHardwareTopology(topoDir);
    if(err != 0)
    {
        errdiag("Error loading hardware: "+topoDir);
        return err;
    }

    QString dataSetDir(dataDir+QString("/data/samples.csv"));
    err = dataSet->loadData(dataSetDir);
    if(err != 0)
    {
        errdiag("Error loading dataset: "+dataSetDir);
        return err;
    }

    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->processData();
        vizWidgets[i]->update();
    }

    visibilityChangedSlot();

    return 0;
}

int MainWindow::selectDataDirectory()
{
    dataDir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select Data Directory"),
                                                  "/Users/chai/Sources/MemAxes/example_data/",
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks);
    if(dataDir.isNull())
        return -1;

    con->append("Selected Data Directory : "+dataDir);

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

void MainWindow::setProgress(int p)
{
    ui->progressBar->setValue(p);
    QApplication::processEvents();
}
