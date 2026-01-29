#pragma once
#include <vtkSystemIncludes.h>
class VAVolumeProperty
{
public:
	VAVolumeProperty();
	~VAVolumeProperty();

	int GetIndependentComponents();
	double GetAmbient(int index);
	double GetDiffuse(int index);
	double GetSpecular(int index);
	double GetSpecularPower(int index);
	int GetShade(int index);
	int GetShade() { return this->GetShade(0); }
private:
	int IndependentComponents;
	int Shade[VTK_MAX_VRCOMP];
	double Ambient[VTK_MAX_VRCOMP];
	double Diffuse[VTK_MAX_VRCOMP];
	double Specular[VTK_MAX_VRCOMP];
	double SpecularPower[VTK_MAX_VRCOMP];
};

