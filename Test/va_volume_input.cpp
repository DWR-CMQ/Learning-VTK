#include "va_volume_input.h"

VolumeInput::VolumeInput(std::shared_ptr<VAVolume> spVolume)
{
	m_spColorFunc = std::make_shared<ColorTransferFunction>();
	m_spOpacityFunc = std::make_shared<OpacityTransferfunction>();
	m_spVolume = spVolume;
	m_spColorTable = std::make_shared<ColorTable>();

	m_spColorFunc->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	m_spColorFunc->AddRGBPoint(-77, 0.5, 0.2, 0.2);
	m_spColorFunc->AddRGBPoint(94, 0.5, 0.5, 0.5);
	m_spColorFunc->AddRGBPoint(179, 0.9, 0.9, 0.9);
	m_spColorFunc->AddRGBPoint(260, 1.0, 1.0, 1.0);
	m_spColorFunc->AddRGBPoint(3071, 0.8, 0.7, 0.6);

	// 创建并配置不透明度传递函数，用于设置体积渲染的不透明度
	m_spOpacityFunc->AddPoint(-3024, 0.0);
	m_spOpacityFunc->AddPoint(-77, 0.0);
	m_spOpacityFunc->AddPoint(94, 0.29);
	m_spOpacityFunc->AddPoint(179, 0.55);
	m_spOpacityFunc->AddPoint(260, 0.84);
	m_spOpacityFunc->AddPoint(3071, 0.875);
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

void VolumeInput::ActivateTransferFunction(int blendMode)
{

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

int VolumeInput::UpdateOpacityTransferFunction(unsigned int component, int blendMode, float samplingDist)
{
	return 1;
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