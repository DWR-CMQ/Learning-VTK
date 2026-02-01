#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vtkMatrix4x4.h>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath);

    /// ¼ÆËã×ÅÉ«Æ÷
    Shader(const char* computePath);

    // activate the shader
    // ------------------------------------------------------------------------
    void use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;
    // ------------------------------------------------------------------------
    void setVec1(const std::string& name, const int* value) const;
    void setVec1(const std::string& name, const float* value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const float* value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec2(const std::string& name, const float(*value)[2]) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const float* value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec3(const std::string& name, const float(*value)[3]) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const float* value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setVec4(const std::string& name, const float(*value)[4]) const;
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, float* matrix) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, float* matrix) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, float* matrix) const;

    void SetUniformMatrix(const std::string& name, vtkMatrix4x4* matrix);

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type);
};
