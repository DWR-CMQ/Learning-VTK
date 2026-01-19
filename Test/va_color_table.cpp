#include "va_color_table.h"

ColorTable::ColorTable(int dataWidth, int dataHeight, int dataDepth)
{
	m_iWidth = dataWidth;
	m_iHeight = dataHeight;
	m_iDepth = dataDepth;
}
int ColorTable::GetMaximumSupportedTextureWidth(vtkOpenGLRenderWindow* renWin, int idealWidth)
{
	if (!this->TextureObject == NULL)
	{
		std::cout << "vtkTextureObject not initialized!" << std::endl;
		return -1;
	}

	// Try to match the next power of two.
	//idealWidth = vtkMath::NearestPowerOfTwo(idealWidth);
	//int const maxWidth = vtkTextureObject::GetMaximumTextureSize(renWin);
	//if (maxWidth < 0)
	//{
	//	std::cout << "Failed to query max texture size! using default 1024." << std::endl;
	//	return 1024;
	//}

	//if (maxWidth >= idealWidth)
	//{
	//	idealWidth = vtkMath::Max(1024, idealWidth);
	//	return idealWidth;
	//}

	//std::cout << "This OpenGL implementation does not support the required "
	//	"texture size of "
	//	<< idealWidth << ", falling back to maximum allowed, " << maxWidth << "."
	//	<< "This may cause an incorrect lookup table mapping." << std::endl;
	//return maxWidth;
}

void ColorTable::ReleaseGraphicsResources(vtkWindow* window)
{

}

bool ColorTable::NeedUpdate(ColorTransferFunction* func, double scalarRange[2], int blendMode, double sampleDistance)
{
	if (func == NULL)
	{
		return false;
	}
	if (scalarRange[0] != this->LastRange[0] || scalarRange[1] != this->LastRange[1])
	{
		this->LastRange[0] = scalarRange[0];
		this->LastRange[1] = scalarRange[1];
		return true;
	}
	return false;
}

void ColorTable::InternalUpdate(ColorTransferFunction* func, int blendMode, double sampleDistance, double unitDistance, int filterValue)
{

}

void ColorTable::ComputeIdealTextureSize(ColorTransferFunction* func, int& width, int& height, vtkOpenGLRenderWindow* renWin)
{
	if (func != NULL)
	{
		width = func->EstimateMinNumberOfSamples(this->LastRange[0], this->LastRange[1]);
		height = 1;
	}
	height = height > 1 ? this->GetMaximumSupportedTextureWidth(renWin, height) : 1;
}

void ColorTable::AllocateTable()
{
	delete[] this->Table;
	this->Table = new float[this->TextureWidth * this->TextureHeight * this->NumberOfColorComponents];
}