#include "va_volume_input.h"
#include <vtkDataArray.h>
VolumeInput::VolumeInput(std::shared_ptr<VAVolume> spVolume)
{
	m_spVolume = spVolume;
	m_spColorTable = std::make_shared<ColorTable>();
	m_spOpacityTable = std::make_shared<OpacityTable>();
}

void VolumeInput::InitializeTransferFunction(int index)
{
	const int transferMode = this->m_spVolume->GetVolumeProperty()->GetTransferFunctionMode();
	switch (transferMode)
	{
	case VAVolumeProperty::TF_2D:
		this->CreateTransferFunction2D(index);
		break;

	case VAVolumeProperty::TF_1D:
	default:
		this->CreateTransferFunction1D(index);
	}
	this->InitializeTransfer = false;
}

void VolumeInput::CreateTransferFunction1D(int index)
{

}

void VolumeInput::CreateTransferFunction2D(int index)
{
}

void VolumeInput::RefreshTransferFunction(int uniformIndex, int blendMode, float samplingDist)
{
	if (this->InitializeTransfer) 
	{
		this->InitializeTransferFunction(uniformIndex);
	}
	this->UpdateTransferFunctions(blendMode, samplingDist);
}

void VolumeInput::ForceTransferInit()
{

}

void VolumeInput::ActivateTransferFunction(Shader* pShader, int blendMode)
{
	int const transferMode = this->m_spVolume->GetVolumeProperty()->GetTransferFunctionMode();
	const int numActiveLuts = 1;
	switch (transferMode)
	{
	case VAVolumeProperty::TF_1D:
		for (int i = 0; i < numActiveLuts; i++)
		{
			this->m_spOpacityTable->Activate();
			this->m_spColorTable->Activate();

			auto iOpacityTexUnit = this->m_spOpacityTable->GetTextureUnit();
			auto iColorTexUnit = this->m_spColorTable->GetTextureUnit();
			// 激活纹理
			pShader->setInt("in_opacityTransferFunc_0[1]", iOpacityTexUnit);
			pShader->setInt("in_colorTransferFunc_0[1]", iColorTexUnit);
		}
		break;
	case VAVolumeProperty::TF_2D:
		break;
	default:
		break;
	}
}

void VolumeInput::DeactivateTransferFunction(int blendMode)
{

}

void VolumeInput::ReleaseGraphicsResources()
{

}

void VolumeInput::UpdateTransferFunctions(int blendMode, float samplingDist)
{
	const int transferMode = m_spVolume->GetVolumeProperty()->GetTransferFunctionMode();
	// 暂时设定为1
	const int numComp = 1;
	switch (transferMode)
	{
	case VAVolumeProperty::TF_1D:
		switch (this->ComponentMode)
		{
		case VolumeInput::INDEPENDENT:
			for (int i = 0; i < numComp; ++i)
			{
				this->UpdateOpacityTransferFunction(i, blendMode, samplingDist);
				this->UpdateColorTransferFunction(i);
			}
			break;
		default: // RGBA or LA
			this->UpdateOpacityTransferFunction(numComp - 1, blendMode, samplingDist);
			this->UpdateColorTransferFunction(0);
		}
		break;

	case VAVolumeProperty::TF_2D:
		break;
	}
}
;
int VolumeInput::UpdateOpacityTransferFunction(unsigned int component, int blendMode, float samplingDist)
{
	auto volumeProperty = m_spVolume->GetVolumeProperty();
	auto opacityTF = volumeProperty->GetOpacityTF(0);

	double componentRange[2];
	if (opacityTF->GetSize() < 1 || this->ScalarOpacityRangeType == VolumeInput::SCALAR)
	{
		for (int i = 0; i < 2; ++i)
		{
			componentRange[i] = m_spVolume->ScalarRange[component][i];
		}
	}
	else
	{
		opacityTF->GetRange(componentRange);
	}

	// Add points only if its not being added before
	if (opacityTF->GetSize() < 1)
	{
		opacityTF->AddPoint(componentRange[0], 0.0);
		opacityTF->AddPoint(componentRange[1], 0.5);
	}

	int filterVal = volumeProperty->GetInterpolationType() == VTK_LINEAR_INTERPOLATION
		? TextureObject::Linear
		: TextureObject::Nearest;

	this->m_spOpacityTable->Update(volumeProperty->GetOpacityTF(component), componentRange, 0, 0, 0, TextureObject::Nearest);
	return 0;
}

int VolumeInput::UpdateColorTransferFunction(unsigned int component)
{
	auto volumeProperty = m_spVolume->GetVolumeProperty();
	auto colorTF = volumeProperty->GetColorTF(0);

	double componentRange[2];
	if (colorTF->GetSize() < 1 || this->ColorRangeType == VolumeInput::SCALAR)
	{
		for (int i = 0; i < 2; ++i)
		{
			componentRange[i] = m_spVolume->ScalarRange[component][i];
		}
	}
	else
	{
		colorTF->GetRange(componentRange);
	}

	// Add points only if its not being added before
	if (colorTF->GetSize() < 1)
	{
		colorTF->AddRGBPoint(componentRange[0], 0.0, 0.0, 0.0);
		colorTF->AddRGBPoint(componentRange[1], 1.0, 1.0, 1.0);
	}

	int filterVal = volumeProperty->GetInterpolationType() == VTK_LINEAR_INTERPOLATION
		? TextureObject::Linear
		: TextureObject::Nearest;

	this->m_spColorTable->Update(volumeProperty->GetColorTF(component),componentRange, 0, 0, 0, TextureObject::Nearest);
	return 0;
}

void VolumeInput::ReleaseGraphicsTransfer1D()
{

}