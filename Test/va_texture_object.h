#pragma once
#include <glad/glad.h>
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
    TextureObject();
    ~TextureObject();

    void CreateTexture();
    void DestroyTexture();
protected:
    unsigned int Handle;    // 纹理对应的句柄
    int NumberOfDimensions;
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int Samples;
    bool UseSRGBColorSpace;

    float MaximumAnisotropicFiltering;

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
};

