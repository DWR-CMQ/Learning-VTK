#pragma once
#include "va_color_transferfunction.h"
#include "va_texture_object.h"
#include <iostream>
class ColorTable
{
public:
	ColorTable(int dataWidth = 1, int dataHeight = 1, int dataDepth = 1);
	~ColorTable();
	int GetMaximumSupportedTextureWidth(int idealWidth);
	void ReleaseGraphicsResources();
	void Update(std::shared_ptr<ColorTransferFunction> func, double scalarRange[2], int blendMode, double sampleDistance, double unitDistance, int filterValue);
	void Activate();
	void Deactivate();
protected:
	virtual bool NeedUpdate(std::shared_ptr<ColorTransferFunction> func, double scalarRange[2], int blendMode, double sampleDistance);
	virtual void InternalUpdate(std::shared_ptr<ColorTransferFunction> func, int blendMode, double sampleDistance, double unitDistance, int filterValue);
	virtual void ComputeIdealTextureSize(std::shared_ptr<ColorTransferFunction> func, int& width, int& height);
	virtual void AllocateTable();

protected:
	double LastRange[2] = { 0.0,0.0 };
	float* Table = nullptr;
	int LastInterpolation = -1;
	int NumberOfColorComponents = 1;
	int TextureWidth = 1024;
	int TextureHeight = 1;
	TextureObject* m_pTextureObject = nullptr;
private:
	int m_iWidth = 0;
	int m_iHeight = 0;
	int m_iDepth = 0;
};

