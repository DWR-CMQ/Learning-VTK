#pragma once
#include <iostream>
#include <string>
#include <vector>

class CommonFunction
{
public:
	static bool CheckDICOMDirectory(const std::string& directoryPath);

	static void HSVToRGB(double h, double s, double v, double* r, double* g, double* b);
	static void HSVToRGB(float h, float s, float v, float* r, float* g, float* b);
	static void HSVToRGB(const double hsv[3], double rgb[3]);
	static void HSVToRGB(const float hsv[3], float rgb[3]);

	static void RGBToHSV(const float rgb[3], float hsv[3]);
	static void RGBToHSV(float r, float g, float b, float* h, float* s, float* v);
	static void RGBToHSV(const double rgb[3], double hsv[3]);
	static void RGBToHSV(double r, double g, double b, double* h, double* s, double* v);

    template <typename T>
    void ToFloat(const T& in1, const T& in2, float(&out)[2])
    {
        out[0] = static_cast<float>(in1);
        out[1] = static_cast<float>(in2);
    }

    template <typename T>
    void ToFloat(const T& in1, const T& in2, const T& in3, float(&out)[3])
    {
        out[0] = static_cast<float>(in1);
        out[1] = static_cast<float>(in2);
        out[2] = static_cast<float>(in3);
    }

    template <typename T>
    void ToFloat(T* in, float* out, int noOfComponents)
    {
        for (int i = 0; i < noOfComponents; ++i)
        {
            out[i] = static_cast<float>(in[i]);
        }
    }

    template <typename T>
    void ToFloat(T(&in)[3], float(&out)[3])
    {
        out[0] = static_cast<float>(in[0]);
        out[1] = static_cast<float>(in[1]);
        out[2] = static_cast<float>(in[2]);
    }

    template <unsigned int N, typename T>
    std::array<float, N> ToFloat(T* in)
    {
        std::array<float, N> out;
        for (size_t i = 0; i < N; i++)
        {
            out[i] = static_cast<float>(in[i]);
        }
        return out;
    }

    template <typename T>
    void ToFloat(T(&in)[2], float(&out)[2])
    {
        out[0] = static_cast<float>(in[0]);
        out[1] = static_cast<float>(in[1]);
    }

    template <typename T>
    void ToFloat(T& in, float& out)
    {
        out = static_cast<float>(in);
    }
    template <typename T>
    void ToFloat(T(&in)[4][2], float(&out)[4][2])
    {
        out[0][0] = static_cast<float>(in[0][0]);
        out[0][1] = static_cast<float>(in[0][1]);
        out[1][0] = static_cast<float>(in[1][0]);
        out[1][1] = static_cast<float>(in[1][1]);
        out[2][0] = static_cast<float>(in[2][0]);
        out[2][1] = static_cast<float>(in[2][1]);
        out[3][0] = static_cast<float>(in[3][0]);
        out[3][1] = static_cast<float>(in[3][1]);
    }

    template <typename T, int SizeX, int SizeY>
    static void CopyMatrixToVector(T* matrix, float* matrixVec, int offset)
    {
        const int MatSize = SizeX * SizeY;
        for (int j = 0; j < MatSize; j++)
        {
            matrixVec[offset + j] = matrix->Element[j / SizeX][j % SizeY];
        }
    }

    template <typename T, int SizeSrc>
    void CopyVector(T* srcVec, T* dstVec, int offset)
    {
        for (int j = 0; j < SizeSrc; j++)
        {
            dstVec[offset + j] = srcVec[j];
        }
    }
};