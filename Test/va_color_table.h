#pragma once
#include "va_color_transferfunction.h"
#include "va_texture_object.h"
#include <vtkOpenGLRenderWindow.h>
class ColorTable
{
public:
	ColorTable(int dataWidth, int dataHeight, int dataDepth);
	~ColorTable();
	int GetMaximumSupportedTextureWidth(vtkOpenGLRenderWindow* renWin, int idealWidth);
	void ReleaseGraphicsResources(vtkWindow* window);

protected:
	virtual bool NeedUpdate(ColorTransferFunction* func, double scalarRange[2], int blendMode, double sampleDistance);
	virtual void InternalUpdate(ColorTransferFunction* func, int blendMode, double sampleDistance, double unitDistance, int filterValue);
	virtual void ComputeIdealTextureSize(ColorTransferFunction* func, int& width, int& height, vtkOpenGLRenderWindow* renWin);
	virtual void AllocateTable();

protected:
	double LastRange[2] = { 0.0,0.0 };
	float* Table = nullptr;
	int LastInterpolation = -1;
	int NumberOfColorComponents = 1;
	int TextureWidth = 1024;
	int TextureHeight = 1;
	vtkTextureObject* TextureObject = nullptr;
private:
	int m_iWidth = 0;
	int m_iHeight = 0;
	int m_iDepth = 0;
};

