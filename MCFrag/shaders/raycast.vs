#version 430 core
#define highp
#define mediump
#define lowp
#define attribute in
#define varying out

uniform vec3 in_cellSpacing[1];
uniform mat4 in_modelViewMatrix;
uniform mat4 in_projectionMatrix;
uniform mat4 in_volumeMatrix[1];
uniform mat4 in_inverseTextureDatasetMatrix[1];
uniform mat4 in_cellToPoint[1];

out mat4 ip_inverseTextureDataAdjusted;

// input
in vec3 in_vertexPos;
// output
out vec3 ip_textureCoords;
out vec3 ip_vertexPos;

void main()
{
    vec4 pos = in_projectionMatrix * in_modelViewMatrix * in_volumeMatrix[0] * vec4(in_vertexPos.xyz, 1.0);
    gl_Position = pos;
    vec3 uvx = sign(in_cellSpacing[0]) * (in_inverseTextureDatasetMatrix[0] * vec4(in_vertexPos, 1.0)).xyz;
    ip_textureCoords = (in_cellToPoint[0] * vec4(uvx, 1.0)).xyz;
    ip_inverseTextureDataAdjusted = in_cellToPoint[0] * in_inverseTextureDatasetMatrix[0];

    ip_vertexPos = in_vertexPos;
}










