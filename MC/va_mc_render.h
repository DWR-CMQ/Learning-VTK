#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "va_mc_shader.h"
#include "va_mc_camera.h"
#include "va_mc_volume.h"
class MCRender
{
public:
	MCRender(vtkImageData* image, glm::ivec2 windowSize, glm::ivec2 fbSize);
	virtual ~MCRender();
	void SetUp();
	void SetUpFBO();
	void LoadShader();
	void SetupUBO(bool is_update = false);
	void Render();

private:
	friend class MCApp;
	GLuint m_iFBOID;
	GLuint m_iFBOTexID;
	GLuint m_iVolumeTex3D;
	GLuint m_iCamUBOID;
	glm::ivec2 m_WindowSize;
	glm::ivec2 m_FrameBufferSize;

	int m_iWorkGroupX, m_iWorkGroupY;
	float alpha_scale;

	std::shared_ptr<MCShader> m_spShader;
	std::shared_ptr<MCCamera> m_spCamera;
	std::shared_ptr<MCVolume> m_spVolume;

	glm::vec3 voxel_size;
	glm::ivec3 tex3D_dim;
	int datasize_bytes, min_val, max_val, max_dataset_val, min_dataset_val;
};

