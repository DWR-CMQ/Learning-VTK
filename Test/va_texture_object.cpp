#include "va_texture_object.h"
#include <iostream>
#include <glad/glad.h>
TextureObject::TextureObject()
{
	UseSRGBColorSpace = false;

	this->WrapS = Repeat;
	this->WrapT = Repeat;
	this->WrapR = Repeat;
	this->MinificationFilter = Nearest;
	this->MagnificationFilter = Nearest;
	this->MinLOD = -1000.0f;
	this->MaxLOD = 1000.0f;
	this->BaseLevel = 0;
	this->MaxLevel = 0;
}

TextureObject::~TextureObject()
{

}


void TextureObject::CreateTexture()
{
	if (this->Handle == 0)
	{
		GLuint tex = 0;
		glGenTextures(1, &tex);
		this->Handle = tex;

		if (this->Target != 0 && this->Target != GL_TEXTURE_BUFFER)
		{
			glBindTexture(this->Target, this->Handle);
			if (this->Target != GL_TEXTURE_2D_MULTISAMPLE)
			{
				glTexParameteri(this->Target, GL_TEXTURE_MIN_FILTER, this->GetMinificationFilterMode(this->MinificationFilter));
				glTexParameteri(this->Target, GL_TEXTURE_MAG_FILTER, this->GetMagnificationFilterMode(this->MagnificationFilter));
				glTexParameteri(this->Target, GL_TEXTURE_WRAP_S, this->GetWrapSMode(this->WrapS));
				glTexParameteri(this->Target, GL_TEXTURE_WRAP_T, this->GetWrapTMode(this->WrapT));

				if (this->Target == GL_TEXTURE_3D)
				{
					glTexParameteri(this->Target, GL_TEXTURE_WRAP_R, this->GetWrapRMode(this->WrapR));
				}
			}

			if (this->Target == GL_TEXTURE_2D)
			{
				glTexParameteri(this->Target, GL_TEXTURE_BASE_LEVEL, this->BaseLevel);
				glTexParameteri(this->Target, GL_TEXTURE_MAX_LEVEL, this->MaxLevel);
			}

			glBindTexture(this->Target, 0);
		}
	}
}

void TextureObject::DestroyTexture()
{
	this->DeActivateTexture();
	if (this->Handle)
	{
		GLuint tex = this->Handle;
		glDeleteTextures(1, &tex);
	}
	this->Handle = 0;
	this->NumberOfDimensions = 0;
	this->Target = 0;
	this->Components = 0;
	this->Width = this->Height = this->Depth = 0;
	this->ResetFormatAndType();
}

void TextureObject::ReleaseGraphicsResources()
{
	if (this->Handle)
	{
		GLuint tex = this->Handle;
		glDeleteTextures(1, &tex);
	
		this->Handle = 0;
		this->NumberOfDimensions = 0;
		this->Target = 0;
		this->InternalFormat = 0;
		this->Format = 0;
		this->Type = 0;
		this->Components = 0;
		this->Width = this->Height = this->Depth = 0;
	}
}

void TextureObject::ResetFormatAndType()
{
	this->Format = 0;
	this->InternalFormat = 0;
	this->Type = 0;
}

void TextureObject::InitializeTextureInternalFormats()
{
	// 0 = none
	// 1 = float
	// 2 = int

	// initialize to zero
	int dtypeCount = sizeof(this->TextureInternalFormats) / sizeof(this->TextureInternalFormats[0]);
	for (int dtype = 0; dtype < dtypeCount; dtype++)
	{
		for (int ctype = 0; ctype < 3; ctype++)
		{
			for (int comp = 0; comp < 5; comp++)
			{
				this->TextureInternalFormats[dtype][ctype][comp] = 0;
			}
		}
	}

	this->TextureInternalFormats[VTK_VOID][0][1] = GL_DEPTH_COMPONENT;

#ifdef GL_R8
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][1] = GL_R8;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][2] = GL_RG8;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][3] = GL_RGB8;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][4] = GL_RGBA8;
#else
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][1] = GL_LUMINANCE;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][2] = GL_LUMINANCE_ALPHA;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][3] = GL_RGB;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][0][4] = GL_RGBA;
#endif

#ifdef GL_R16
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][0][1] = GL_R16;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][0][2] = GL_RG16;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][0][3] = GL_RGB16;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][0][4] = GL_RGBA16;
#endif

#ifdef GL_R8_SNORM
	this->TextureInternalFormats[VTK_SIGNED_CHAR][0][1] = GL_R8_SNORM;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][0][2] = GL_RG8_SNORM;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][0][3] = GL_RGB8_SNORM;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][0][4] = GL_RGBA8_SNORM;
#endif

#ifdef GL_R16_SNORM
	this->TextureInternalFormats[VTK_SHORT][0][1] = GL_R16_SNORM;
	this->TextureInternalFormats[VTK_SHORT][0][2] = GL_RG16_SNORM;
	this->TextureInternalFormats[VTK_SHORT][0][3] = GL_RGB16_SNORM;
	this->TextureInternalFormats[VTK_SHORT][0][4] = GL_RGBA16_SNORM;
#endif

#ifdef GL_R8I
	this->TextureInternalFormats[VTK_SIGNED_CHAR][2][1] = GL_R8I;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][2][2] = GL_RG8I;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][2][3] = GL_RGB8I;
	this->TextureInternalFormats[VTK_SIGNED_CHAR][2][4] = GL_RGBA8I;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][2][1] = GL_R8UI;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][2][2] = GL_RG8UI;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][2][3] = GL_RGB8UI;
	this->TextureInternalFormats[VTK_UNSIGNED_CHAR][2][4] = GL_RGBA8UI;

	this->TextureInternalFormats[VTK_SHORT][2][1] = GL_R16I;
	this->TextureInternalFormats[VTK_SHORT][2][2] = GL_RG16I;
	this->TextureInternalFormats[VTK_SHORT][2][3] = GL_RGB16I;
	this->TextureInternalFormats[VTK_SHORT][2][4] = GL_RGBA16I;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][2][1] = GL_R16UI;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][2][2] = GL_RG16UI;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][2][3] = GL_RGB16UI;
	this->TextureInternalFormats[VTK_UNSIGNED_SHORT][2][4] = GL_RGBA16UI;

	this->TextureInternalFormats[VTK_INT][2][1] = GL_R32I;
	this->TextureInternalFormats[VTK_INT][2][2] = GL_RG32I;
	this->TextureInternalFormats[VTK_INT][2][3] = GL_RGB32I;
	this->TextureInternalFormats[VTK_INT][2][4] = GL_RGBA32I;
	this->TextureInternalFormats[VTK_UNSIGNED_INT][2][1] = GL_R32UI;
	this->TextureInternalFormats[VTK_UNSIGNED_INT][2][2] = GL_RG32UI;
	this->TextureInternalFormats[VTK_UNSIGNED_INT][2][3] = GL_RGB32UI;
	this->TextureInternalFormats[VTK_UNSIGNED_INT][2][4] = GL_RGBA32UI;
#endif

#ifdef GL_R32F
	this->TextureInternalFormats[VTK_FLOAT][1][1] = GL_R32F;
	this->TextureInternalFormats[VTK_FLOAT][1][2] = GL_RG32F;
	this->TextureInternalFormats[VTK_FLOAT][1][3] = GL_RGB32F;
	this->TextureInternalFormats[VTK_FLOAT][1][4] = GL_RGBA32F;

	this->TextureInternalFormats[VTK_SHORT][1][1] = GL_R32F;
	this->TextureInternalFormats[VTK_SHORT][1][2] = GL_RG32F;
	this->TextureInternalFormats[VTK_SHORT][1][3] = GL_RGB32F;
	this->TextureInternalFormats[VTK_SHORT][1][4] = GL_RGBA32F;
#endif
}

unsigned int TextureObject::GetFormat(int dataType, int numComps, bool shaderSupportsTextureInt)
{
	if (!this->Format)
	{
		this->Format = this->GetDefaultFormat(dataType, numComps, shaderSupportsTextureInt);
	}
	return this->Format;
}

bool TextureObject::Create1DTextureFromRaw(unsigned int width, int numComps, int dataType, void* data)
{
	this->GetInternalFormat(dataType, numComps, false);
	this->GetFormat(dataType, numComps, false);
	this->GetDataType(dataType);
	if (!this->InternalFormat || !this->Format || !this->Type)
	{
		std::cout << "Failed to determine texture parameters." << std::endl;
		return false;
	}

	GLenum target = GL_TEXTURE_1D;
	this->Target = target;
	this->Components = numComps;
	// 暂定为1 需要window类传值
	this->Width = 1;
	this->Height = 1;
	this->Depth = 1;
	this->NumberOfDimensions = 1;
	this->Bind();
	this->CreateTexture();

	glTexImage1D(this->Target, 0, this->InternalFormat, static_cast<GLsizei>(this->Width), 0,
					this->Format, this->Type, static_cast<const GLvoid*>(data));
	return true;
}

bool TextureObject::Create2DTextureFromRaw(unsigned int width, unsigned int height, int numComps, int dataType, void* data)
{
	this->GetInternalFormat(dataType, numComps, false);
	this->GetFormat(dataType, numComps, false);
	this->GetDataType(dataType);
	if (!this->InternalFormat || !this->Format || !this->Type)
	{
		std::cout << "Failed to determine texture parameters." << std::endl;
		return false;
	}

	GLenum target = GL_TEXTURE_2D;
	this->Target = target;
	this->Components = numComps;
	this->Width = width;
	this->Height = height;
	this->Depth = 1;
	this->NumberOfDimensions = 2;
	this->Bind();
	this->CreateTexture();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(this->Target, 0, this->InternalFormat, static_cast<GLsizei>(this->Width), static_cast<GLsizei>(this->Height), 0,
							this->Format, this->Type, static_cast<const GLvoid*>(data));
	return true;
}

bool TextureObject::Create3DTextureFromRaw(unsigned int width, unsigned int height, unsigned int depth, int numComps, int dataType, void* data)
{
	this->GetInternalFormat(dataType, numComps, false);
	this->GetFormat(dataType, numComps, false);
	this->GetDataType(dataType);
	if (!this->InternalFormat || !this->Format || !this->Type)
	{
		std::cout << "Failed to determine texture parameters." << std::endl;
		return false;
	}

	GLenum target = GL_TEXTURE_3D;
	this->Target = target;
	this->Components = numComps;
	this->Width = width;
	this->Height = height;
	this->Depth = depth;
	this->NumberOfDimensions = 3;
	this->Bind();
	this->CreateTexture();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(this->Target, 0, this->InternalFormat, static_cast<GLsizei>(this->Width), static_cast<GLsizei>(this->Height), static_cast<GLsizei>(this->Depth),
					0, this->Format, this->Type, static_cast<const GLvoid*>(data));

	return true;
}

// datatype
int TextureObject::GetDefaultDataType(int dataType)
{
	switch (dataType)
	{
	case VTK_SIGNED_CHAR:
		return GL_BYTE;
	case VTK_UNSIGNED_CHAR:
		return GL_UNSIGNED_BYTE;
	case VTK_SHORT:
		return GL_SHORT;
	case VTK_UNSIGNED_SHORT:
		return GL_UNSIGNED_SHORT;
	case VTK_INT:
		return GL_INT;
	case VTK_UNSIGNED_INT:
		return GL_UNSIGNED_INT;
	case VTK_FLOAT:
	case VTK_VOID:
		return GL_FLOAT;
	}
	return 0;
}

int TextureObject::GetDataType(int dataType)
{
	if (!this->Type)
	{
		this->Type = this->GetDefaultDataType(dataType);
	}
	return this->Type;
}

void TextureObject::SetDataType(unsigned int dataType)
{
	if (this->Type != dataType)
	{
		this->Type = dataType;
	}
}

// format
unsigned int TextureObject::GetDefaultFormat(int dataType, int numComps, bool shaderSupportsTextureInt)
{
	if (dataType == VTK_VOID)
	{
		return GL_DEPTH_COMPONENT;
	}

	if (this->SupportsTextureInteger && shaderSupportsTextureInt &&
		(dataType == VTK_SIGNED_CHAR || dataType == VTK_UNSIGNED_CHAR || dataType == VTK_SHORT ||
			dataType == VTK_UNSIGNED_SHORT || dataType == VTK_INT || dataType == VTK_UNSIGNED_INT))
	{
		switch (numComps)
		{
		case 1:
			return GL_RED_INTEGER;
		case 2:
			return GL_RG_INTEGER;
			//case 3:
			//	return GL_RGB_INTEGER_EXT;
			//case 4:
			//	return GL_RGBA_INTEGER_EXT;
		}
	}
	else
	{
		switch (numComps)
		{
		case 1:
			return GL_RED;
		case 2:
			return GL_RG;
		case 3:
			return GL_RGB;
		case 4:
			return GL_RGBA;
		}
	}
	return GL_RGB;
}

unsigned int TextureObject::GetInternalFormat(int dataType, int numComps, bool shaderSupportsTextureInt)
{
	if (this->InternalFormat)
	{
		return this->InternalFormat;
	}

	// pre-condition
	if (dataType == VTK_VOID && numComps != 1)
	{
		std::cout << "Depth component texture must have 1 component only (" << numComps << " requested" << std::endl;
		this->InternalFormat = 0;
		return this->InternalFormat;
	}

	this->InternalFormat = this->GetDefaultInternalFormat(dataType, numComps, shaderSupportsTextureInt);
	if (this->InternalFormat == 0)
	{
		std::cout << "Failed to get internal format for texture!" << std::endl;
		std::cout << "Unable to find suitable internal format for T="
			<< dataType << " NC=" << numComps << " SSTI=" << shaderSupportsTextureInt;
	}
	return this->InternalFormat;
}

unsigned int TextureObject::GetDefaultInternalFormat(int dataType, int numComps, bool shaderSupportsTextureInt)
{
	GLenum result = 0;
	if (shaderSupportsTextureInt)
	{
		result = this->GetDefaultTextureInternalFormat(dataType, numComps, true, false, this->UseSRGBColorSpace);
		if (result == 0)
		{
			std::cout << "Unsupported internal texture type!" << std::endl;
		}
		return result;
	}

	result = this->GetDefaultTextureInternalFormat(dataType, numComps, false, false, this->UseSRGBColorSpace);
	if (result != 0)
	{
		return result;
	}

	result = this->GetDefaultTextureInternalFormat(dataType, numComps, false, true, this->UseSRGBColorSpace);
	if (result == 0)
	{
		std::cout << "Unsupported internal texture type!" << std::endl;
		std::cout << "Unable to find suitable internal format for T = "
			<< dataType << " NC=" << numComps << " SSTI=" << shaderSupportsTextureInt << std::endl;
	}
	return result;
}

int TextureObject::GetDefaultTextureInternalFormat(int dataType, int numComponents, bool needInteger, bool needFloat, bool needSRGB)
{
	int dtypeCount = sizeof(this->TextureInternalFormats) / sizeof(this->TextureInternalFormats[0]);
	if (dataType >= dtypeCount)
	{
		return 0;
	}

	if (needInteger)
	{
		return this->TextureInternalFormats[dataType][2][numComponents];
	}
	if (needFloat)
	{
		return this->TextureInternalFormats[dataType][1][numComponents];
	}
	int result = this->TextureInternalFormats[dataType][0][numComponents];
	if (needSRGB)
	{
		switch (result)
		{
		case GL_RGB:
			result = GL_SRGB;
		case GL_RGBA:
			result = GL_SRGB_ALPHA;
		case GL_RGB8:
			result = GL_SRGB8;
		case GL_RGBA8:
			result = GL_SRGB8_ALPHA8;
		default:
			break;
		}
	}
	return result;
}

void TextureObject::Bind()
{
	glBindTexture(this->Target, this->Handle);
}

unsigned int TextureObject::GetMinificationFilterMode(int filterType)
{
	switch (filterType)
	{
	case Nearest:
		return GL_NEAREST;
	case Linear:
		return GL_LINEAR;
	case NearestMipmapNearest:
		return GL_NEAREST_MIPMAP_NEAREST;
	case NearestMipmapLinear:
		return GL_NEAREST_MIPMAP_LINEAR;
	case LinearMipmapNearest:
		return GL_LINEAR_MIPMAP_NEAREST;
	case LinearMipmapLinear:
		return GL_LINEAR_MIPMAP_LINEAR;
	default:
		return GL_NEAREST;
	}
}

unsigned int TextureObject::GetMagnificationFilterMode(int filterType)
{
	switch (filterType)
	{
	case Nearest:
		return GL_NEAREST;
	case Linear:
		return GL_LINEAR;
	default:
		return GL_NEAREST;
	}
}

unsigned int TextureObject::SetWrapSMode(int wrapType)
{
	return WrapS;
}

unsigned int TextureObject::GetWrapSMode(int wrapType)
{
	switch (wrapType)
	{
	case ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case Repeat:
		return GL_REPEAT;
#ifdef GL_CLAMP_TO_BORDER
	case ClampToBorder:
		return GL_CLAMP_TO_BORDER;
#endif
	case MirroredRepeat:
		return GL_MIRRORED_REPEAT;
	default:
		return GL_CLAMP_TO_EDGE;
	}
}

unsigned int TextureObject::SetWrapTMode(int wrapType)
{
	return WrapT;
}

unsigned int TextureObject::GetWrapTMode(int wrapType)
{
	return this->GetWrapSMode(wrapType);
}

unsigned int TextureObject::SetWrapRMode(int wrapType)
{
	return WrapR;
}

unsigned int TextureObject::GetWrapRMode(int wrapType)
{
	return this->GetWrapSMode(wrapType);
}

void TextureObject::ActivateTexture(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
}

void TextureObject::DeActivateTexture()
{

}

void TextureObject::SetInternalFormat(unsigned int glInternalFormat)
{
	if (this->InternalFormat != glInternalFormat)
	{
		this->InternalFormat = glInternalFormat;
	}
}

void TextureObject::SetFormat(unsigned int glFormat)
{
	if (this->Format != glFormat)
	{
		this->Format = glFormat;
	}
}

void TextureObject::SetMinificationFilterMode(int filterType)
{
	MinificationFilter = filterType;
}

void TextureObject::SetMagnificationFilterMode(int filterType)
{
	MagnificationFilter = filterType;
}