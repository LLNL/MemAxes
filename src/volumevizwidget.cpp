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

#include "volumevizwidget.h"

#include <QMouseEvent>

#include <cmath>
using namespace std;

#include "util.h"

VolumeVizWidget::VolumeVizWidget(QWidget *parent)
    : QVTKWidget(parent)
{
    this->setMinimumSize(400,400);

    // dims and data
    width = 45;
    height = 45;
    depth = 45;

    minAlpha = 0.01;
    midAlpha = 0.5;
    maxAlpha = 1.00;

    volumeData.resize(width*height*depth);
    volumeData.fill(0);

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    processed = false;

    // Create everything anew
    imageImport = NULL;
    volRendMapper = vtkSmartPointer<vtkVolumeTextureMapper3D>::New();
    opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    volumeProps = vtkSmartPointer<vtkVolumeProperty>::New();
    volume = vtkSmartPointer<vtkVolume>::New();
    renderer = vtkSmartPointer<vtkRenderer>::New();

    //this->GetRenderWindow()->AddRenderer(renderer);

    selectionChangedSlot();
}

VolumeVizWidget::~VolumeVizWidget()
{
    this->GetRenderWindow()->RemoveRenderer(renderer);
}

void VolumeVizWidget::setDataSet(DataSetObject *iData)
{
    dataset = iData;
    processData();
}

#define ROWMAJOR(_x,_y,_z,_w,_h) _z*_w*_h+_y*_w+_x

void VolumeVizWidget::processData()
{
    selectionChangedSlot();
    processed = true;
}

void VolumeVizWidget::selectionChangedSlot()
{
    if(!processed)
        return;

    volumeData.fill(0);

    qreal v;
    int x,y,z,map3D;
    for(int d=0; d<dataset->size(); d++)
    {
        int elem = 0;

        int mapdim = dataset->at(d)->meta.indexOf("map3D");
        int xdim = dataset->at(d)->meta.indexOf("xidx");
        int ydim = dataset->at(d)->meta.indexOf("yidx");
        int zdim = dataset->at(d)->meta.indexOf("zidx");
        int wdim = dataset->at(d)->meta.indexOf("latency");

        for(QVector<qreal>::Iterator val=dataset->at(d)->begin;
            val!=dataset->at(d)->end;
            val+=dataset->at(d)->numDimensions, elem++)
        {
            if(dataset->selectionDefined() && !dataset->at(d)->selected(elem))
                continue;

            map3D = *(val+mapdim);

            if(!map3D)
                continue;

            x = *(val+xdim);
            y = *(val+ydim);
            z = *(val+zdim);
            v = *(val+wdim);

            if(x >= width || y >= height || z >= depth)
                continue;

            if(x < 0 || y < 0 || z < 0)
                continue;

            qreal newv = volumeData[ROWMAJOR(x,y,z,width,height)] + v;

            volumeData[ROWMAJOR(x,y,z,width,height)] = newv;
        }
    }

    minVal = std::numeric_limits<double>::max();
    maxVal = 0;
    qreal numVals = width*height*depth;
    qreal sumVals = 0;
    for(int i=0; i<numVals; i++)
    {
        minVal = min(minVal,(qreal)volumeData[i]);
        maxVal = max(maxVal,(qreal)volumeData[i]);
        sumVals += volumeData[i];
    }

    midVal = sumVals / numVals;

    emit minValSet(minVal);
    emit midValSet(midVal);
    emit maxValSet(maxVal);

    confVtk();

    this->update();
}

void VolumeVizWidget::updateTransferFunction()
{
    if(!processed)
        return;

    // Color Transfer functions
    colorTransferFunction->RemoveAllPoints();
    colorTransferFunction->AddRGBPoint(minVal,0,0,1);
    colorTransferFunction->AddRGBPoint(midVal,0,1,0);
    colorTransferFunction->AddRGBPoint(maxVal,1,0,0);

    // Opacity Transfer function
    opacityFunction->RemoveAllPoints();
    opacityFunction->AddPoint(minVal,minAlpha);
    opacityFunction->AddPoint(midVal,midAlpha);
    opacityFunction->AddPoint(maxVal,maxAlpha);

    this->update();
}

void VolumeVizWidget::confVtk()
{
    if(renderer)
        this->GetRenderWindow()->RemoveRenderer(renderer);
    this->GetRenderWindow()->AddRenderer(renderer);

    imageImport = vtkSmartPointer<vtkImageImport>::New();

    // QVector<float> to vtkImageData
    imageImport->SetDataSpacing(1, 1, 1);
    imageImport->SetDataOrigin(0, 0, 0);
    imageImport->SetWholeExtent(0, width-1, 0, height-1, 0, depth-1);
    imageImport->SetDataExtentToWholeExtent();
    imageImport->SetDataScalarTypeToFloat();
    imageImport->SetNumberOfScalarComponents(1);

    volRendMapper->SetSampleDistance(0.2);

    imageImport->SetImportVoidPointer((void*)volumeData.constData());
    //volRendMapper->SetInput(imageImport->GetOutput());
    volRendMapper->SetInputData(imageImport->GetOutput());

    // Set properties for volume rendering
    volumeProps->SetColor(colorTransferFunction);
    volumeProps->SetScalarOpacity(opacityFunction);
    volumeProps->SetInterpolationTypeToNearest();
    volumeProps->ShadeOff();

    volume->SetProperty(volumeProps);
    volume->SetMapper(volRendMapper);

    renderer->RemoveVolume(volume);
    renderer->AddVolume(volume);

    imageImport->Update();
    volRendMapper->Update();
    volume->Update();
}

void VolumeVizWidget::setMinVal(double val)
{
    minVal = val;
    updateTransferFunction();
}

void VolumeVizWidget::setMidVal(double val)
{
    midVal = val;
    updateTransferFunction();
}

void VolumeVizWidget::setMaxVal(double val)
{
    maxVal = val;
    updateTransferFunction();
}

void VolumeVizWidget::setMinOpacity(int val)
{
    minAlpha = (qreal)val / (qreal)100.0;
    updateTransferFunction();
}

void VolumeVizWidget::setMidOpacity(int val)
{
    midAlpha = (qreal)val / (qreal)100.0;
    updateTransferFunction();
}

void VolumeVizWidget::setMaxOpacity(int val)
{
    maxAlpha = (qreal)val / (qreal)100.0;
    updateTransferFunction();
}
