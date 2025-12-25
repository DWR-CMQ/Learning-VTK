#pragma once

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <glad/glad.h>


struct TextureInfo
{
    GLuint id = 0;
    int width = 0;
    int height = 0;
    int depth = 0;
    GLenum internalFormat = GL_R8;
    GLenum format = GL_RED;
    GLenum type = GL_UNSIGNED_BYTE;
};

class MCVolume
{
public:

public:
	MCVolume(vtkSmartPointer<vtkImageData> input);
    TextureInfo GetDataType();
	GLuint ConvertImageDataToTexture3D();
    vtkSmartPointer<vtkImageData> GetImageData();

private:
    TextureInfo m_stInfo;
    vtkSmartPointer<vtkImageData> m_spImageData;
};

