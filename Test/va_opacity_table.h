#pragma once
#include "va_opacity_transferfunction.h"
#include "va_texture_object.h"
#include <iostream>
class OpacityTable
{
public:
	OpacityTable(int dataWidth = 1, int dataHeight = 1, int dataDepth = 1);
	~OpacityTable();
	int GetMaximumSupportedTextureWidth(int idealWidth);
	void ReleaseGraphicsResources();
	void Update(std::shared_ptr<OpacityTransferfunction> func, double scalarRange[2], int blendMode, double sampleDistance, double unitDistance, int filterValue);
	void Activate();
	void Deactivate();
protected:
	virtual bool NeedUpdate(std::shared_ptr<OpacityTransferfunction> func, double scalarRange[2], int blendMode, double sampleDistance);
	virtual void InternalUpdate(std::shared_ptr<OpacityTransferfunction> func, int blendMode, double sampleDistance, double unitDistance, int filterValue);
	virtual void ComputeIdealTextureSize(std::shared_ptr<OpacityTransferfunction> func, int& width, int& height);
	virtual void AllocateTable();

protected:
	double LastRange[2] = { 0.0,0.0 };
	float* Table = nullptr;
	int LastInterpolation = -1;
	int NumberOfColorComponents = 1;
	int TextureWidth = 1024;
	int TextureHeight = 1;
	TextureObject* m_pTextureObject = nullptr;
	int LastBlendMode = TextureObject::MAXIMUM_INTENSITY_BLEND;
	double LastSampleDistance = 1.0;
private:
	int m_iWidth = 0;
	int m_iHeight = 0;
	int m_iDepth = 0;
};

