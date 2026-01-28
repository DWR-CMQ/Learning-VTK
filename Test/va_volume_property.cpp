#include "va_volume_property.h"

VAVolumeProperty::VAVolumeProperty()
{
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