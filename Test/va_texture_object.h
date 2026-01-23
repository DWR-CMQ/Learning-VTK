#pragma once
#include <glad/glad.h>
#include <vtkType.h>
class TextureObject
{
public:
    // DepthTextureCompareFunction values.
    enum
    {
        Lequal = 0, // r=R<=Dt ? 1.0 : 0.0
        Gequal,     // r=R>=Dt ? 1.0 : 0.0
        Less,       // r=R<D_t ? 1.0 : 0.0
        Greater,    // r=R>Dt ? 1.0 : 0.0
        Equal,      // r=R==Dt ? 1.0 : 0.0
        NotEqual,   // r=R!=Dt ? 1.0 : 0.0
        AlwaysTrue, //  r=1.0 // WARNING "Always" is macro defined in X11/X.h...
        Never,      // r=0.0
        NumberOfDepthTextureCompareFunctions
    };

    // Wrap values.
    enum
    {
        ClampToEdge = 0,
        Repeat,
        MirroredRepeat,
        ClampToBorder,
        NumberOfWrapModes
    };

    // MinificationFilter values.
    enum
    {
        Nearest = 0,
        Linear,
        NearestMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapNearest,
        LinearMipmapLinear,
        NumberOfMinificationModes
    };

    // depth/color format
    enum
    {
        Native = 0, // will try to match with the depth buffer format.
        Fixed8,
        Fixed16,
        Fixed24,
        Fixed32,
        Float16,
        Float32,
        NumberOfDepthFormats
    };

public:
    TextureObject(int dataType);
    ~TextureObject();

    void InitializeTextureInternalFormats();
    void CreateTexture();
    void DestroyTexture();

    bool Create1DTexture(int numComps, bool shaderSupportsTextureInt);
    bool Create2DTexture(unsigned int width, unsigned int height, int numComps, bool shaderSupportsTextureInt);
    bool Create3DTexture(unsigned int width, unsigned int height, unsigned int depth, int numComps, bool shaderSupportsTextureInt);
    int GetDefaultDataType(int dataType);
    unsigned int GetDefaultFormat(int dataType, int numComps, bool shaderSupportsTextureInt);
    unsigned int GetDefaultInternalFormat(int dataType, int numComps, bool shaderSupportsTextureInt);
    int GetDefaultTextureInternalFormat(int dataType, int numComponents, bool needInteger, bool needFloat, bool needSRGB);
    unsigned int GetInternalFormat(int dataType, int numComps, bool shaderSupportsTextureInt);

    void Bind();
    unsigned int GetMinificationFilterMode(int filterType);
    unsigned int GetMagnificationFilterMode(int filterType);
    unsigned int GetWrapSMode(int wrapType);
    unsigned int GetWrapTMode(int wrapType);
    unsigned int GetWrapRMode(int wrapType);

    void ActivateTexture(int unit);
    void DeActivateTexture();

protected:
    unsigned int Handle;    // 纹理对应的句柄
    int NumberOfDimensions;
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int Samples;
    bool UseSRGBColorSpace;
    int m_iDataType;

    float MaximumAnisotropicFiltering;
    bool RequireTextureInteger;
    bool SupportsTextureInteger;
    bool RequireTextureFloat;
    bool SupportsTextureFloat;

    int TextureInternalFormats[VTK_OBJECT + 1][3][5];

    unsigned int Target;         // GLenum
    unsigned int Format;         // GLenum
    unsigned int InternalFormat; // GLenum
    unsigned int Type;           // GLenum
    int Components;

    int WrapS;
    int WrapT;
    int WrapR;
    int MinificationFilter;
    int MagnificationFilter;

    float MinLOD;
    float MaxLOD;
    int BaseLevel;
    int MaxLevel;
};