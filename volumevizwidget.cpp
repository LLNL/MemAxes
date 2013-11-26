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
    xdim = 9;
    ydim = 10;
    zdim = 11;
    wdim = 4;

    minAlpha = 0.05;
    midAlpha = 0.5;
    maxAlpha = 1.00;

    // Initialize vtk pointers
    imageImport = vtkSmartPointer<vtkImageImport>::New();
    volRendMapper = vtkSmartPointer<vtkVolumeTextureMapper3D>::New();
    opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    volumeProps = vtkSmartPointer<vtkVolumeProperty>::New();
    volume = vtkSmartPointer<vtkVolume>::New();
    renderer = vtkSmartPointer<vtkRenderer>::New();

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->GetRenderWindow()->AddRenderer(renderer);
}

VolumeVizWidget::~VolumeVizWidget()
{
}

void VolumeVizWidget::setData(DataObject *iData)
{
    data = iData;
    processData();
}

#define ROWMAJOR(_x,_y,_z,_w,_h) _z*_w*_h+_y*_w+_x

void VolumeVizWidget::processData()
{
    QVector<qreal>::Iterator p;

    qreal minx = *(data->begin+xdim);
    qreal maxx = minx;

    qreal miny = *(data->begin+ydim);
    qreal maxy = miny;

    qreal minz = *(data->begin+zdim);
    qreal maxz = minz;

    minVal = *(data->begin+wdim);
    midVal = minVal;
    maxVal = minVal;

    int x,y,z;
    qreal v;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        x = *(p+xdim);
        y = *(p+ydim);
        z = *(p+zdim);
        v = *(p+wdim);

        minx = min((qreal)x,minx);
        maxx = max((qreal)x,maxx);

        miny = min((qreal)y,miny);
        maxy = max((qreal)y,maxy);

        minz = min((qreal)z,minz);
        maxz = max((qreal)z,maxz);

        minVal = min(v,minVal);
        maxVal = max(v,maxVal);
    }

    midVal = (maxVal - minVal) / 2.0;

    emit minValSet(minVal);
    emit midValSet(midVal);
    emit maxValSet(maxVal);

    width = maxx - minx + 1;
    height = maxy - miny + 1;
    depth = maxz - minz + 1;

    cout << width << "x" << height << "x" << depth << endl;

    volumeData.clear();
    volumeData.resize(width*height*depth);
    volumeData.fill(0);

    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        x = *(p+xdim);
        y = *(p+ydim);
        z = *(p+zdim);
        v = *(p+wdim);

        volumeData[ROWMAJOR(x,y,z,width,height)] += v;
    }

    // QVector<float> to vtkImageData
    imageImport->SetDataSpacing(1, 1, 1);
    imageImport->SetDataOrigin(0, 0, 0);
    imageImport->SetWholeExtent(0, width-1, 0, height-1, 0, depth-1);
    imageImport->SetDataExtentToWholeExtent();
    imageImport->SetDataScalarTypeToFloat();
    imageImport->SetNumberOfScalarComponents(1);
    imageImport->SetImportVoidPointer((void*)volumeData.constData());
    imageImport->Update();

    volRendMapper->SetInput(imageImport->GetOutput());
    volRendMapper->SetSampleDistance(0.2);

    updateTransferFunction();

    // Set properties for volume rendering
    volumeProps->SetColor(colorTransferFunction);
    volumeProps->SetScalarOpacity(opacityFunction);
    volumeProps->SetInterpolationTypeToLinear();
    volumeProps->ShadeOff();

    volume->SetProperty(volumeProps);
    volume->SetMapper(volRendMapper);

    renderer->AddVolume(volume);
    renderer->ResetCamera();
}

void VolumeVizWidget::selectionChangedSlot()
{
    volumeData.clear();
    volumeData.resize(width*height*depth);
    volumeData.fill(0);

    int elem;
    QVector<qreal>::Iterator p;
    int x,y,z;
    qreal v;
    bool seldef = data->selectionDefined();
    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(seldef && data->selected(elem) == 0)
            continue;

        if(!data->visible(elem))
            continue;

        x = *(p+xdim);
        y = *(p+ydim);
        z = *(p+zdim);
        v = *(p+wdim);

        volumeData[ROWMAJOR(x,y,z,width,height)] += v;
    }

    imageImport->SetImportVoidPointer((void*)volumeData.constData());
}

void VolumeVizWidget::updateTransferFunction()
{
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

void VolumeVizWidget::setXDim(int val)
{
    xdim = val;
    selectionChangedSlot();
    update();
}

void VolumeVizWidget::setYDim(int val)
{
    ydim = val;
    selectionChangedSlot();
    update();
}

void VolumeVizWidget::setZDim(int val)
{
    zdim = val;
    selectionChangedSlot();
    update();
}

void VolumeVizWidget::setWDim(int val)
{
    wdim = val;
    selectionChangedSlot();
    update();
}
