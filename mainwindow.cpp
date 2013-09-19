#include <iostream>
using namespace std;

#include <QTimer>
#include <QFileDialog>
#include <QTextStream>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("CorrelaViz"));

    // Create viz widgets
    CorrelationMatrixViz *correlationMatrixViz = new CorrelationMatrixViz();
    ui->correlMatrixLayout->addWidget(correlationMatrixViz);

    ScatterPlotViz *scatterPlotViz = new ScatterPlotViz();
    ui->scatterPlotLayout->addWidget(scatterPlotViz);

    ParallelCoordinatesViz *parallelCoordinatesViz = new ParallelCoordinatesViz();
    ui->parallelCoordinatesLayout->addWidget(parallelCoordinatesViz);

    // Make UI connections
    // mainwindow
    connect(ui->actionImport_Data, SIGNAL(triggered()),this,SLOT(importData()));

    // correlationmatrix
    connect(ui->minSlider, SIGNAL(valueChanged(int)), correlationMatrixViz, SLOT(setMin(int)));
    connect(ui->maxSlider, SIGNAL(valueChanged(int)), correlationMatrixViz, SLOT(setMax(int)));
    connect(correlationMatrixViz, SIGNAL(selectedDims(int,int)), scatterPlotViz, SLOT(setDims(int,int)));

    // pcoords
    connect(ui->selOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setSelOpacity(int)));
    connect(ui->unselOpacity, SIGNAL(valueChanged(int)), parallelCoordinatesViz, SLOT(setUnselOpacity(int)));

    // Add to viz widgets
    vizWidgets.push_back(correlationMatrixViz);
    vizWidgets.push_back(scatterPlotViz);
    vizWidgets.push_back(parallelCoordinatesViz);

    for(int i=0; i<vizWidgets.size(); i++)
    {
        connect(vizWidgets[i], SIGNAL(repaintAll()), this, SLOT(repaintAllVizWidgets()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::repaintAllVizWidgets()
{
    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->repaint();
    }
}

int MainWindow::importData()
{
    // Open a file using a dialog
    QString dataFileName = QFileDialog::getOpenFileName(this, tr("Import Data"),
                                                     "",
                                                     tr("Files (*.*)"));

    DataObject *dobj = parseCSVFile(dataFileName);
    if(dobj == NULL)
        return -1;

    dataObjects.push_back(dobj);

    for(int i=0; i<vizWidgets.size(); i++)
    {
        vizWidgets[i]->setData(dobj);
        vizWidgets[i]->processViz();
    }

    repaintAllVizWidgets();

    return 0;
}

DataObject* MainWindow::parseCSVFile(QString dataFileName)
{
    DataObject *dobj = new DataObject();

    // Open the file
    QFile dataFile(dataFileName);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
         return NULL;

    // Create text stream
    QTextStream dataStream(&dataFile);
    QString line;
    QStringList lineValues;
    long long elemid = 0;

    // Get metadata from first line
    line = dataStream.readLine();
    dobj->meta = line.split(',');
    dobj->numDimensions = dobj->meta.size();

    // Get data
    while(!dataStream.atEnd())
    {
        line = dataStream.readLine();
        lineValues = line.split(',');

        if(lineValues.size() != dobj->numDimensions)
        {
            cerr << "ERROR: element dimensions do not match metadata!" << endl;
            cerr << "At element " << elemid << endl;
            return NULL;
        }

        for(int i=0; i<lineValues.size(); i++)
        {
            dobj->vals.push_back(lineValues[i].toDouble());
        }

        elemid++;
    }

    // Close and return
    dataFile.close();

    dobj->init();

    return dobj;
}

