#pragma once

#include <iostream>
#include "va_color_transferfunction.h"
#include "va_opacity_transferfunction.h"
#include "va_color_table.h"
#include "va_opacity_table.h"
#include "va_volume.h"
#include "va_shader.h"
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

	enum TFRangeType
	{
		SCALAR = 0, // default
		NATIVE
	};

	int ComponentMode = INDEPENDENT;
public:
	VolumeInput(std::shared_ptr<VAVolume> spVolume);

	void RefreshTransferFunction(int uniformIndex, int blendMode, float samplingDist);
	void ForceTransferInit();

	void ActivateTransferFunction(Shader* pShader, int blendMode);
	void DeactivateTransferFunction(int blendMode);

	void ReleaseGraphicsResources();

	void InitializeTransferFunction(int index);
	void CreateTransferFunction1D(int index);
	void CreateTransferFunction2D(int index);

	void UpdateTransferFunctions(int blendMode, float samplingDist);
	int UpdateOpacityTransferFunction(unsigned int component, int blendMode, float samplingDist);
	int UpdateColorTransferFunction( unsigned int component);

	void ReleaseGraphicsTransfer1D();
private:

	std::shared_ptr<ColorTable> m_spColorTable;
	std::shared_ptr<OpacityTable> m_spOpacityTable;
	std::shared_ptr<VAVolume> m_spVolume;
	bool InitializeTransfer = true;
	int ColorRangeType = 0;           
	int ScalarOpacityRangeType = 0;   
};

