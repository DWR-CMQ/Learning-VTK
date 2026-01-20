#include "va_render.h"

Render::Render()
{
	m_uiVao = 0;
	m_uiVbo = 0;
}

Render::~Render()
{
	glDeleteVertexArrays(1, &m_uiVao);
	glDeleteBuffers(1, &m_uiVbo);
	m_uiVao = 0;
	m_uiVbo = 0;
}

