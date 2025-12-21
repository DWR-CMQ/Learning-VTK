#pragma once

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <glad/glad.h>

class MCVolume
{
public:
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
public:
	MCVolume(vtkSmartPointer<vtkImageData> input);
    bool GetDataType();
	GLuint ConvertImageDataToTexture3D();


private:
    TextureInfo m_stInfo;
    vtkSmartPointer<vtkImageData> m_spImageData;
};

