#include "va_volume_input.h"

VolumeInput::VolumeInput()
{
	m_spColorTable = std::make_shared<ColorTransferFunction>();
	m_spOpacityTable = std::make_shared<OpacityTransferfunction>();

	m_spColorTable->AddRGBPoint(-3024, 0.0, 0.0, 0.0);
	m_spColorTable->AddRGBPoint(-77, 0.5, 0.2, 0.2);
	m_spColorTable->AddRGBPoint(94, 0.5, 0.5, 0.5);
	m_spColorTable->AddRGBPoint(179, 0.9, 0.9, 0.9);
	m_spColorTable->AddRGBPoint(260, 1.0, 1.0, 1.0);
	m_spColorTable->AddRGBPoint(3071, 0.8, 0.7, 0.6);

	// 创建并配置不透明度传递函数，用于设置体积渲染的不透明度
	m_spOpacityTable->AddPoint(-3024, 0.0);
	m_spOpacityTable->AddPoint(-77, 0.0);
	m_spOpacityTable->AddPoint(94, 0.29);
	m_spOpacityTable->AddPoint(179, 0.55);
	m_spOpacityTable->AddPoint(260, 0.84);
	m_spOpacityTable->AddPoint(3071, 0.875);
}

void VolumeInput::RefreshTransferFunction(int uniformIndex, int blendMode, float samplingDist)
{

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

void VolumeInput::InitializeTransferFunction(int index)
{

}

void VolumeInput::CreateTransferFunction1D(int index)
{

}

void VolumeInput::UpdateTransferFunctions(int blendMode, float samplingDist)
{

}

int VolumeInput::UpdateOpacityTransferFunction(unsigned int component, int blendMode, float samplingDist)
{
	return 1;
}

int VolumeInput::UpdateColorTransferFunction(unsigned int component)
{
	return 1;
}

void VolumeInput::ReleaseGraphicsTransfer1D()
{

}