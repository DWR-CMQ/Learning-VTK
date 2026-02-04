#include "va_color_table.h"
#include <vtkMath.h>
#include <glad/glad.h>
ColorTable::ColorTable(int dataWidth, int dataHeight, int dataDepth)
{
	m_iWidth = dataWidth;
	m_iHeight = dataHeight;
	m_iDepth = dataDepth;
}
ColorTable::~ColorTable()
{
	delete[] this->Table;
}

int ColorTable::GetMaximumSupportedTextureWidth(int idealWidth)
{
	if (this->m_pTextureObject == NULL)
	{
		std::cout << "vtkTextureObject not initialized!" << std::endl;
		return -1;
	}

	// Try to match the next power of two.
	idealWidth = vtkMath::NearestPowerOfTwo(idealWidth);
	int maxWidth = -1;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxWidth);
	if (maxWidth < 0)
	{
		std::cout << "Failed to query max texture size! using default 1024." << std::endl;
		return 1024;
	}

	if (maxWidth >= idealWidth)
	{
		idealWidth = vtkMath::Max(1024, idealWidth);
		return idealWidth;
	}

	std::cout << "This OpenGL implementation does not support the required "
		"texture size of "
		<< idealWidth << ", falling back to maximum allowed, " << maxWidth << "."
		<< "This may cause an incorrect lookup table mapping." << std::endl;
	return maxWidth;
}

void ColorTable::ReleaseGraphicsResources()
{
	if (this->m_pTextureObject != NULL)
	{
		this->m_pTextureObject->ReleaseGraphicsResources();
		this->m_pTextureObject = nullptr;
	}
}

void ColorTable::Update(std::shared_ptr<ColorTransferFunction> func, double scalarRange[2], int blendMode, double sampleDistance, double unitDistance, int filterValue)
{
	if (func == nullptr)
	{
		return;
	}
	if (this->m_pTextureObject == nullptr)
	{
		this->m_pTextureObject = new TextureObject();
	}

	if (this->NeedUpdate(func, scalarRange, blendMode, sampleDistance))
	{
		int idealW = 1024;
		int newHeight = 1;
		this->ComputeIdealTextureSize(func, idealW, newHeight);
		int const newWidth = this->GetMaximumSupportedTextureWidth(idealW);
		if (this->Table == NULL || this->TextureWidth != newWidth || this->TextureHeight != newHeight)
		{
			this->TextureWidth = newWidth;
			this->TextureHeight = newHeight;
			this->AllocateTable();
		}

		this->InternalUpdate(func, blendMode, sampleDistance, unitDistance, filterValue);
		this->LastInterpolation = filterValue;
	}

	if (this->LastInterpolation != filterValue)
	{
		this->LastInterpolation = filterValue;
		this->m_pTextureObject->SetMagnificationFilterMode(filterValue);
		this->m_pTextureObject->SetMinificationFilterMode(filterValue);
	}
}

bool ColorTable::NeedUpdate(std::shared_ptr<ColorTransferFunction> func, double scalarRange[2], int blendMode, double sampleDistance)
{
	if (func == nullptr)
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

void ColorTable::InternalUpdate(std::shared_ptr<ColorTransferFunction> func, int blendMode, double sampleDistance, double unitDistance, int filterValue)
{
	if (func == nullptr)
	{
		return;
	}
	func->GetTable(this->LastRange[0], this->LastRange[1], this->TextureWidth, this->Table);
	this->m_pTextureObject->SetWrapSMode(TextureObject::ClampToEdge);
	this->m_pTextureObject->SetWrapTMode(TextureObject::ClampToEdge);
	this->m_pTextureObject->SetMagnificationFilterMode(filterValue);
	this->m_pTextureObject->SetMinificationFilterMode(filterValue);
	this->m_pTextureObject->Create2DTextureFromRaw(this->TextureWidth, 1, this->NumberOfColorComponents, VTK_FLOAT, this->Table);
}

void ColorTable::ComputeIdealTextureSize(std::shared_ptr<ColorTransferFunction> func, int& width, int& height)
{
	if (func != NULL)
	{
		width = func->EstimateMinNumberOfSamples(this->LastRange[0], this->LastRange[1]);
		height = 1;
	}
	height = height > 1 ? this->GetMaximumSupportedTextureWidth(height) : 1;
}

void ColorTable::AllocateTable()
{
	delete[] this->Table;
	this->Table = new float[this->TextureWidth * this->TextureHeight * this->NumberOfColorComponents];
}