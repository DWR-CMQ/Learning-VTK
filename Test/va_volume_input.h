#pragma once

#include <iostream>
#include "va_color_transferfunction.h"
#include "va_opacity_transferfunction.h"
#include "va_texture_object.h"
class VolumeInput
{
public:
	enum ComponentMode
	{
		INVALID = 0,
		INDEPENDENT = 1,
		LA = 2,
		RGBA = 4
	};
	int ComponentMode = INDEPENDENT;
public:
	VolumeInput();

	void RefreshTransferFunction(int uniformIndex, int blendMode, float samplingDist);
	void ForceTransferInit();

	void ActivateTransferFunction( int blendMode);
	void DeactivateTransferFunction(int blendMode);

	void ReleaseGraphicsResources();

	void InitializeTransferFunction(int index);
	void CreateTransferFunction1D(int index);

	void UpdateTransferFunctions(int blendMode, float samplingDist);
	int UpdateOpacityTransferFunction(unsigned int component, int blendMode, float samplingDist);
	int UpdateColorTransferFunction( unsigned int component);

	void ReleaseGraphicsTransfer1D();
private:
	std::shared_ptr<ColorTransferFunction> m_spColorTable;
	std::shared_ptr<OpacityTransferfunction> m_spOpacityTable; 

	std::shared_ptr<TextureObject> m_spColorTableTexture;
	std::shared_ptr<TextureObject> m_spOpacityTableTexture;
};

