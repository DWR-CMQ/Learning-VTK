#pragma once
#include <vtkSystemIncludes.h>
class VAVolumeProperty
{
public:
	VAVolumeProperty();
	~VAVolumeProperty();

public:
	int Shade[VTK_MAX_VRCOMP];
	double Ambient[VTK_MAX_VRCOMP];
	double Diffuse[VTK_MAX_VRCOMP];
	double Specular[VTK_MAX_VRCOMP];
	double SpecularPower[VTK_MAX_VRCOMP];
};

