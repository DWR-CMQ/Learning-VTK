#pragma once
#include <vtkSystemIncludes.h>
#include "va_color_transferfunction.h"
#include "va_opacity_transferfunction.h"
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

	void SetColorTF(int index, ColorTransferFunction* function);
	ColorTransferFunction* GetColorTF(int index);

	void SetOpacityTF(int index, OpacityTransferfunction* function);
	OpacityTransferfunction* GetOpacityTF(int index);

	void SetInterpolationType(int type);
	void SetInterpolationTypeToNearest() { this->SetInterpolationType(VTK_NEAREST_INTERPOLATION); }
	void SetInterpolationTypeToLinear() { this->SetInterpolationType(VTK_LINEAR_INTERPOLATION); }
	int GetInterpolationType();

	enum TransferMode
	{
		TF_1D = 0,
		TF_2D
	};
	void SetTransferFunctionMode(int mode);
	int GetTransferFunctionMode();
private:
	int IndependentComponents;
	int Shade[VTK_MAX_VRCOMP];
	double Ambient[VTK_MAX_VRCOMP];
	double Diffuse[VTK_MAX_VRCOMP];
	double Specular[VTK_MAX_VRCOMP];
	double SpecularPower[VTK_MAX_VRCOMP];

	int TransferFunctionMode;
	ColorTransferFunction* ColorTF[1];
	OpacityTransferfunction* OpacityTF[1];
	int InterpolationType;
};

