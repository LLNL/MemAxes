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
    xdim = 12;
    ydim = 13;
    zdim = 14;
    wdim = 7;

    minAlpha = 0.05;
    midAlpha = 0.5;
    maxAlpha = 1.00;

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

    // HARD CODE
    width = 45;
    height = 45;
    depth = 45;

    qreal minx = std::numeric_limits<double>::max();
    qreal maxx = 0;

    qreal miny = std::numeric_limits<double>::max();
    qreal maxy = 0;

    qreal minz = std::numeric_limits<double>::max();
    qreal maxz = 0;

    minVal = std::numeric_limits<double>::max();
    maxVal = 0;

    int x,y,z,map3D;
    qreal v;
    for(p=data->begin; p!=data->end; p+=data->numDimensions)
    {
        map3D = *(p+mapdim);

        if(!map3D)
            continue;

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

    selectionChangedSlot();
}

void VolumeVizWidget::selectionChangedSlot()
{
    if(renderer)
        this->GetRenderWindow()->RemoveRenderer(renderer);

    imageImport = vtkSmartPointer<vtkImageImport>::New();
    volRendMapper = vtkSmartPointer<vtkVolumeTextureMapper3D>::New();
    volume = vtkSmartPointer<vtkVolume>::New();
    renderer = vtkSmartPointer<vtkRenderer>::New();

    this->GetRenderWindow()->AddRenderer(renderer);

    volumeData.clear();
    volumeData.resize(width*height*depth);
    volumeData.fill(0);

    int elem;
    QVector<qreal>::Iterator p;
    int x,y,z,map3D;
    qreal v;
    for(p=data->begin, elem=0; p!=data->end; p+=data->numDimensions, elem++)
    {
        if(data->skip(elem))
            continue;

        map3D = *(p+mapdim);

        if(!map3D)
            continue;

        x = *(p+xdim);
        y = *(p+ydim);
        z = *(p+zdim);
        v = *(p+wdim);

        if(x >= width || y >= height || z >= depth)
            continue;

        if(x < 0 || y < 0 || z < 0)
            continue;

        volumeData[ROWMAJOR(x,y,z,width,height)] += v;
    }

    updateTransferFunction();

    // QVector<float> to vtkImageData
    imageImport->SetDataSpacing(1, 1, 1);
    imageImport->SetDataOrigin(0, 0, 0);
    imageImport->SetWholeExtent(0, width-1, 0, height-1, 0, depth-1);
    imageImport->SetDataExtentToWholeExtent();
    imageImport->SetDataScalarTypeToFloat();
    imageImport->SetNumberOfScalarComponents(1);

    volRendMapper->SetSampleDistance(0.2);

    imageImport->SetImportVoidPointer((void*)volumeData.constData());
    imageImport->Update();
    volRendMapper->SetInput(imageImport->GetOutput());

    // Set properties for volume rendering
    volumeProps->SetColor(colorTransferFunction);
    volumeProps->SetScalarOpacity(opacityFunction);
    volumeProps->SetInterpolationTypeToNearest();
    volumeProps->ShadeOff();

    volume->SetProperty(volumeProps);
    volume->SetMapper(volRendMapper);

    renderer->RemoveVolume(volume);
    renderer->AddVolume(volume);
    renderer->ResetCamera();

    this->update();
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

void VolumeVizWidget::setMapDim(int val)
{
    mapdim = val;

    if(processed)
        selectionChangedSlot();
}

void VolumeVizWidget::setXDim(int val)
{
    xdim = val;

    if(processed)
        selectionChangedSlot();
}

void VolumeVizWidget::setYDim(int val)
{
    ydim = val;

    if(processed)
        selectionChangedSlot();
}

void VolumeVizWidget::setZDim(int val)
{
    zdim = val;

    if(processed)
        selectionChangedSlot();
}

void VolumeVizWidget::setWDim(int val)
{
    wdim = val;

    if(processed)
        selectionChangedSlot();
}
