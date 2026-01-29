#include "va_volume_property.h"

VAVolumeProperty::VAVolumeProperty()
{
    this->IndependentComponents = 1;
    for (int i = 0; i < VTK_MAX_VRCOMP; i++)
    {
        this->Shade[i] = 0;
        this->Ambient[i] = 0.1;
        this->Diffuse[i] = 0.7;
        this->Specular[i] = 0.2;
        this->SpecularPower[i] = 10.0;
    }
}

VAVolumeProperty::~VAVolumeProperty()
{

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