#ifndef GRID3D_H
#define GRID3D_H

#include <QVTKWidget.h>

#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkImageImport.h>
#include <vtkVolumeTextureMapper3D.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

#include "dataobject.h"

class VolumeVizWidget : public QVTKWidget
{
    Q_OBJECT

public:
    VolumeVizWidget(QWidget *parent = 0);
    ~VolumeVizWidget();

public:
    void setData(DataObject* iData);

protected:
    virtual void processData();

public slots:
    void processSelection();
    void setMinVal(double val);
    void setMidVal(double val);
    void setMaxVal(double val);
    void setMinOpacity(int val);
    void setMidOpacity(int val);
    void setMaxOpacity(int val);

signals:
    void minValSet(double val);
    void midValSet(double val);
    void maxValSet(double val);

private:
    void updateTransferFunction();

private:
    QVector<float> volumeData;

protected:
    DataObject *data;
    bool processed;

    int margin;
    QColor bgColor;

private:

    vtkSmartPointer<vtkImageImport> imageImport;
    vtkSmartPointer<vtkVolumeTextureMapper3D> volRendMapper;
    vtkSmartPointer<vtkPiecewiseFunction> opacityFunction;
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction;
    vtkSmartPointer<vtkVolumeProperty> volumeProps;
    vtkSmartPointer<vtkVolume> volume;
    vtkSmartPointer<vtkRenderer> renderer;

    int xdim;
    int ydim;
    int zdim;
    int valdim;

    qreal minVal;
    qreal midVal;
    qreal maxVal;

    qreal minAlpha;
    qreal midAlpha;
    qreal maxAlpha;

    int width;
    int height;
    int depth;
};

#endif // GRID3D_H
