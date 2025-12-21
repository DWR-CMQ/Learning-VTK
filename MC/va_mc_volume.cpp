#include "va_mc_volume.h"
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include "va_mc_shader.h"

MCVolume::MCVolume(vtkSmartPointer<vtkImageData> input)
{
    m_spImageData = input;
}

bool MCVolume::GetDataType()
{
    if (m_spImageData == NULL)
    {
        std::cout << "ConvertImageDataToVoid Input is invalid!" << std::endl;
        return false;
    }

    vtkDataArray* scalars = m_spImageData->GetPointData()->GetScalars();
    if (scalars == NULL)
    {
        std::cout << "ConvertImageDataToVoid scalars is invalid!" << std::endl;
        return false;
    }

    int numComponents = scalars->GetNumberOfComponents();
    int dataType = scalars->GetDataType();

    // 设置默认格式
    m_stInfo.format = GL_RED;
    m_stInfo.internalFormat = GL_R8;
    m_stInfo.type = GL_UNSIGNED_BYTE;

    int dims[3];
    m_spImageData->GetDimensions(dims);
    m_stInfo.width = dims[0];
    m_stInfo.height = dims[1];
    m_stInfo.depth = dims[2];

    // 根据组件数设置格式
    switch (numComponents)
    {
    case 1:
        m_stInfo.format = GL_RED;
        break;
    case 2:
        m_stInfo.format = GL_RG;
        break;
    case 3:
        m_stInfo.format = GL_RGB;
        break;
    case 4:
        m_stInfo.format = GL_RGBA;
        break;
    default:
        std::cout << "ConvertImageDataToVoid numComponets is invalid!" << std::endl;
        break;
    }

    // 根据数据类型设置内部格式和类型
    switch (dataType)
    {
    case VTK_UNSIGNED_CHAR:
        m_stInfo.type = GL_UNSIGNED_BYTE;
        if (numComponents == 1) m_stInfo.internalFormat = GL_R8;
        else if (numComponents == 2) m_stInfo.internalFormat = GL_RG8;
        else if (numComponents == 3) m_stInfo.internalFormat = GL_RGB8;
        else if (numComponents == 4) m_stInfo.internalFormat = GL_RGBA8;
        break;

    case VTK_FLOAT:
        m_stInfo.type = GL_FLOAT;
        if (numComponents == 1) m_stInfo.internalFormat = GL_R32F;
        else if (numComponents == 2) m_stInfo.internalFormat = GL_RG32F;
        else if (numComponents == 3) m_stInfo.internalFormat = GL_RGB32F;
        else if (numComponents == 4) m_stInfo.internalFormat = GL_RGBA32F;
        break;

    case VTK_SHORT:
        m_stInfo.type = GL_SHORT;
        if (numComponents == 1) m_stInfo.internalFormat = GL_R16_SNORM;
        else if (numComponents == 2) m_stInfo.internalFormat = GL_RG16_SNORM;
        else if (numComponents == 3) m_stInfo.internalFormat = GL_RGB16_SNORM;
        else if (numComponents == 4) m_stInfo.internalFormat = GL_RGBA16_SNORM;
        break;

    case VTK_UNSIGNED_SHORT:
        m_stInfo.type = GL_UNSIGNED_SHORT;
        if (numComponents == 1) m_stInfo.internalFormat = GL_R16;
        else if (numComponents == 2) m_stInfo.internalFormat = GL_RG16;
        else if (numComponents == 3) m_stInfo.internalFormat = GL_RGB16;
        else if (numComponents == 4) m_stInfo.internalFormat = GL_RGBA16;
        break;
    default:
        std::cout << "ConvertImageDataToVoid dataType is invalid!" << std::endl;
        break;
    }
}

GLuint MCVolume::ConvertImageDataToTexture3D()
{
    void* data = m_spImageData->GetScalarPointer();
    GLuint id = 0;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload it to the GPU, assuming that input volume data inherently doesn't have any row alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage3D(GL_TEXTURE_3D,
        0,
        m_stInfo.internalFormat,
        m_stInfo.width,
        m_stInfo.height,
        m_stInfo.depth,
        0,
        m_stInfo.format,
        m_stInfo.type,
        data);
    glBindTexture(GL_TEXTURE_3D, 0);
    return id;
}