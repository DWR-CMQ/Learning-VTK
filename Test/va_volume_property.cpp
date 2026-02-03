#include "va_volume_property.h"

VAVolumeProperty::VAVolumeProperty()
{
    this->IndependentComponents = 1;
    this->InterpolationType = VTK_NEAREST_INTERPOLATION;

    for (int i = 0; i < VTK_MAX_VRCOMP; i++)
    {
        this->Shade[i] = 0;
        this->Ambient[i] = 0.1;
        this->Diffuse[i] = 0.7;
        this->Specular[i] = 0.2;
        this->SpecularPower[i] = 10.0;
    }
    this->TransferFunctionMode = VAVolumeProperty::TF_1D;
}

VAVolumeProperty::~VAVolumeProperty()
{

}

void VAVolumeProperty::SetColorTF(int index, ColorTransferFunction* function)
{
    if (this->ColorTF[index] != function)
    {
        this->ColorTF[index] = function;

        this->TransferFunctionMode = VAVolumeProperty::TF_1D;
    }
}

ColorTransferFunction* VAVolumeProperty::GetColorTF(int index)
{
    if (this->ColorTF[index] == nullptr)
    {
        this->ColorTF[index] = new ColorTransferFunction();
        this->ColorTF[index]->AddRGBPoint(0, 0.0, 0.0, 0.0);
        this->ColorTF[index]->AddRGBPoint(1024, 1.0, 1.0, 1.0);
    }
    return this->ColorTF[index];
}

void VAVolumeProperty::SetOpacityTF(int index, OpacityTransferfunction* function)
{
    if (this->OpacityTF[index] != function)
    {
        this->OpacityTF[index] = function;

        this->TransferFunctionMode = VAVolumeProperty::TF_1D;
    }
}

OpacityTransferfunction* VAVolumeProperty::GetOpacityTF(int index)
{
    if (this->OpacityTF[index] == nullptr)
    {
        this->OpacityTF[index] = new OpacityTransferfunction();
        this->OpacityTF[index]->AddPoint(0, 1.0);
        this->OpacityTF[index]->AddPoint(1024, 1.0);
    }

    return this->OpacityTF[index];
}

void VAVolumeProperty::SetInterpolationType(int type)
{
    InterpolationType = type;
}

int VAVolumeProperty::GetInterpolationType()
{
    return InterpolationType;
}

int VAVolumeProperty::GetIndependentComponents()
{
    return this->IndependentComponents;
}

double VAVolumeProperty::GetAmbient(int index)
{
    return this->Ambient[index];
}

double VAVolumeProperty::GetDiffuse(int index)
{
    return this->Diffuse[index];
}

double VAVolumeProperty::GetSpecular(int index)
{
    return this->Specular[index];
}

double VAVolumeProperty::GetSpecularPower(int index)
{
    return this->SpecularPower[index];
}

int VAVolumeProperty::GetShade(int index)
{
    return this->Shade[index];
}

void VAVolumeProperty::SetTransferFunctionMode(int mode)
{
    TransferFunctionMode = mode;
}

int VAVolumeProperty::GetTransferFunctionMode()
{
    return TransferFunctionMode;
}