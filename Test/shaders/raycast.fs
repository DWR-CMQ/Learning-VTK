#version 430 core

in vec3 ip_textureCoords;
in vec3 ip_vertexPos;
vec4 g_fragColor = vec4(0.0);

vec3 g_dirStep;
float g_lengthStep = 0.0;
vec4 g_srcColor;
vec4 g_eyePosObj;
bool g_exit;
bool g_skip;
float g_currentT;
float g_terminatePointMax;

vec3 g_rayOrigin;
vec3 g_rayTermination;

vec3 g_dataPos;
vec3 g_terminatePos;

float g_jitterValue = 0.0;
uniform sampler3D in_volume[1];
uniform vec4 in_volume_scale[1];
uniform vec4 in_volume_bias[1];
uniform int in_noOfComponents;

uniform sampler2D in_depthSampler;
uniform mat4 in_volumeMatrix[1];
uniform mat4 in_inverseVolumeMatrix[1];
uniform mat4 in_textureDatasetMatrix[1];
uniform mat4 in_inverseTextureDatasetMatrix[1];
uniform mat4 in_textureToEye[1];
uniform vec3 in_texMin[1];
uniform vec3 in_texMax[1];

uniform vec3 in_eyePosObjs[1];
uniform mat4 in_cellToPoint[1];
uniform mat4 in_projectionMatrix;
uniform mat4 in_inverseProjectMatrix;
uniform mat4 in_modelViewMatrix;
uniform mat4 in_inverseModelViewMatrix;
in mat4 ip_inverseTextureDataAdjusted;

uniform vec3 in_cellStep[1];
uniform vec2 in_scalarRange[4];
uniform vec3 in_cellSpacing[1];

uniform float in_sampleDistance;
uniform vec2 in_windowLowerLeftCorner;
uniform vec2 in_inverseOriginalWindowSize;
uniform vec2 in_inverseWindowSize;
uniform vec3 in_textureExtentsMax;
uniform vec3 in_textureExtentsMin;

uniform vec3 in_diffuse[4];
uniform vec3 in_ambient[4];
uniform vec3 in_specular[4];
uniform float in_shininess[4];

vec3 g_rayJitter = vec3(0.0);
uniform vec2 in_averageIPRange;
uniform bool in_twoSidedLighting;
uniform vec3 in_lightAmbientColor[1];
uniform vec3 in_lightDiffuseColor[1];
uniform vec3 in_lightSpecularColor[1];
vec4 g_lightPosObj[1];
vec3 g_ldir[1];
vec3 g_vdir[1];
vec3 g_h[1];

const float g_opacityThreshold = 1.0 - 1.0 / 255.0;
#define EPSILON 0.001

// output
out vec4 FragColor;

struct Hit
{
    float tmin;
    float tmax;
};

struct Ray
{
    vec3 origin;
    vec3 dir;
    vec3 invDir;
};

bool BBoxInterset(const vec3 boxMin, const vec3 boxMax, const Ray r, out Hit hit)
{
    vec3 tbot = r.invDir * (boxMin - r.origin);
    vec3 ttop = r.invDir * (boxMax - r.origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);
    hit.tmin = t0;
    hit.tmax = t1;
    return t1 > max(t0, 0.0);
}

void safe_0_vector(inout Ray ray)
{
    if(abs(ray.dir.x) < EPSILON)
    {
        ray.dir.x = sign(ray.dir.x) * EPSILON;
    }
    if(abs(ray.dir.y) < EPSILON)
    {
        ray.dir.y = sign(ray.dir.y) * EPSILON;
    }
    if(abs(ray.dir.z) < EPSILON)
    {
        ray.dir.z = sign(ray.dir.z) * EPSILON;
    }
}

uniform sampler2D in_colorTransferFunc_0[1];
uniform sampler2D in_opacityTransferFunc_0[1];

float computeOpacity(vec4 scalar)
{
    return texture(in_opacityTransferFunc_0[0], vec2(scalar.w, 0)).r;
}

vec4 computeGradient(in vec3 texPos, in int c, in sampler3D volume, in int index)
{
    vec3 g1;
    vec3 g2;
    vec3 xvec = vec3(in_cellStep[index].x, 0.0,0.0);
    vec3 yvec = vec3(0.0, in_cellStep[index].x,0.0);
    vec3 zvec = vec3(0.0, 0.0,in_cellStep[index].z);
    vec3 texPosPvec[3];
    texPosPvec[0] = texPos + xvec;
    texPosPvec[1] = texPos + yvec;
    texPosPvec[2] = texPos + zvec;
    vec3 texPosNvec[3];
    texPosNvec[0] = texPos - xvec;
    texPosNvec[1] = texPos - yvec;
    texPosNvec[2] = texPos - zvec;

    g1.x = texture(volume, vec3(texPosPvec[0]))[c];
    g1.y = texture(volume, vec3(texPosPvec[1]))[c];
    g1.z = texture(volume, vec3(texPosPvec[2]))[c];
    g2.x = texture(volume, vec3(texPosNvec[0]))[c];
    g2.y = texture(volume, vec3(texPosNvec[1]))[c];
    g2.z = texture(volume, vec3(texPosNvec[2]))[c];

    g1 = g1 * in_volume_scale[index][c] + in_volume_bias[index][c];
    g2 = g2 * in_volume_scale[index][c] + in_volume_bias[index][c];
    return vec4((g1 - g2) / in_cellSpacing[index], -1.0);
}

vec4 computeLighting(vec4 color, int component, float label)
{
    vec4 finalColor = vec4(0.0);
    int lightingComponent = component;
    vec4 shading_gradient = computeGradient(g_dataPos, component, in_volume[0], 0);

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 normal = shading_gradient.xyz;
    float normalLength = length(normal);
    if(normalLength > 0.0)
    {
        normal = normalize(normal);
    }
    else
    {
        normal = vec3(0.0,0.0,0.0);    
    }
    float nDotL = dot(normal, -g_ldir[0]);
    vec3 r = normalize(2.0 * nDotL * normal + g_ldir[0]);
    float vDotR = dot(r, -g_vdir[0]);
    if(nDotL < 0.0 && in_twoSidedLighting)
    {
        nDotL = -nDotL;
    }

    if(nDotL > 0.0)
    {
        diffuse = nDotL * in_diffuse[lightingComponent] * in_lightDiffuseColor[0] * color.rgb;
        vDotR = max(vDotR, 0.0);
        specular = pow(vDotR, in_shininess[lightingComponent]) * 
                    in_specular[lightingComponent] * in_lightSpecularColor[0];
    }
    finalColor.xyz = in_ambient[lightingComponent] * color.rgb + diffuse + specular;
    finalColor.a = color.a;
    return finalColor;
}

vec4 computeColor(vec4 scalar, float opacity)
{
    return clamp(computeLighting(vec4(texture(in_colorTransferFunc_0[0], vec2(scalar.w, 0.0)).xyz, opacity), 0,0.0),0.0,1.0);
}

vec3 computeRayDirection()
{
    return normalize(ip_vertexPos.xyz - in_eyePosObjs[0].xyz);
}

uniform float in_scale;
uniform float in_bias;

vec4 WindowToNDC(const float xCoord, const float yCoord, const float zCoord)
{
    vec4 NDCCoord = vec4(0.0, 0.0, 0.0, 1.0);
    NDCCoord.x = (xCoord - in_windowLowerLeftCorner.x) * 2.0 * in_inverseWindowSize.x - 1.0;
    NDCCoord.y = (yCoord - in_windowLowerLeftCorner.y) * 2.0 * in_inverseWindowSize.y - 1.0;
    NDCCoord.z = (2.0 * zCoord - (gl_DepthRange.near + gl_DepthRange.far)) / gl_DepthRange.diff;
    return NDCCoord;
}

vec4 NDCToWindow(const float xNDC, const float yNDC, const float zNDC)
{
    vec4 WinCoord = vec4(0.0, 0.0, 0.0, 1.0);
    WinCoord.x = (xNDC + 1.0f) / (2.0f * in_inverseWindowSize.x) + in_windowLowerLeftCorner.x;
    WinCoord.y = (yNDC + 1.0f) / (2.0f * in_inverseWindowSize.y) + in_windowLowerLeftCorner.y;
    WinCoord.z = (zNDC * gl_DepthRange.diff + gl_DepthRange.near + gl_DepthRange.far) / 2.0f;
    return WinCoord;
}

vec3 ClampToSampleLocation(vec3 start, vec3 step, vec3 pos, bool ceiling)
{
    vec3 offset = pos - start;
    float stepLength = length(step);
    float dist = dot(offset, step / step);
    if(dist < 0.0)
    {
        return start;
    }

    float steps = dist / stepLength;
    if(abs(mod(steps, 1.0f)) > 1e-5)
    {
        if(ceiling)
        {
            steps = ceil(steps);
        }
        else
        {
            steps = floor(steps);
        }
    }
    return start + steps * step;
}

void initializeRayCast()
{
    g_fragColor = vec4(0.0);
    g_dirStep = vec3(0.0);
    g_srcColor = vec4(0.0);
    g_exit = false;
    g_rayOrigin = ip_textureCoords.xyz;
    vec3 rayDir = computeRayDirection();
    vec2 fragTexCoord = (gl_FragCoord.xy - in_windowLowerLeftCorner)*in_inverseWindowSize;
    g_dirStep = (ip_inverseTextureDataAdjusted * vec4(rayDir, 0.0)).xyz * in_sampleDistance;
    g_lengthStep = length(g_dirStep);

    float jitterValue = 0.0;
    g_rayJitter = g_dirStep;
    g_rayOrigin += g_rayJitter;

    g_skip = false;
    g_lightPosObj[0] = vec4(in_eyePosObjs[0], 1.0);
    g_ldir[0] = normalize(g_lightPosObj[0].xyz - ip_vertexPos);
    g_vdir[0] = normalize(in_eyePosObjs[0].xyz - ip_vertexPos);
    g_h[0] = normalize(g_ldir[0] + g_vdir[0]);

    bool stop = false;
    g_terminatePointMax = 0.0;
    vec4 l_depthValue = texture(in_depthSampler, fragTexCoord);
    // Depth test
    if(gl_FragCoord.z >= l_depthValue.x)
    {
        discard;
    }

    fragTexCoord = (gl_FragCoord.xy - in_windowLowerLeftCorner) * in_inverseOriginalWindowSize;
    vec4 rayTermination = WindowToNDC(gl_FragCoord.x, gl_FragCoord.y, l_depthValue.x);
    rayTermination = ip_inverseTextureDataAdjusted * 
                    in_inverseVolumeMatrix[0] * 
                    in_inverseModelViewMatrix * 
                    in_inverseProjectMatrix * 
                    rayTermination;
    g_rayTermination = rayTermination.xyz / rayTermination.w;
    g_dataPos = g_rayOrigin;
    g_terminatePos = g_rayTermination;
    g_terminatePointMax = length(g_terminatePos.xyz - g_dataPos.xyz) / length(g_dirStep);
    g_currentT = 0.0;
    g_jitterValue = jitterValue;
}

vec4 castRay(const float zStart, const float zEnd)
{
    while(!g_exit)
    {
        g_skip = false;
        if(!g_skip)
        {
            vec4 scalar;
            scalar = texture(in_volume[0], g_dataPos);
            scalar.r = scalar.r * in_volume_scale[0].r + in_volume_bias[0].r;
            scalar = vec4(scalar.r);
            g_srcColor = vec4(0.0);
            g_srcColor.a = computeOpacity(scalar);
            if(g_srcColor.a > 0.0)
            {
                g_srcColor = computeColor(scalar, g_srcColor.a);
                g_srcColor.rgb *= g_srcColor.a;
                g_fragColor = (1.0f - g_fragColor.a) * g_srcColor + g_fragColor;
            }
        }

        g_dataPos += g_dirStep;
        if(any(greaterThan(max(g_dirStep, vec3(0.0)) * (g_dataPos - in_texMax[0]), vec3(0.0))) ||
            any(greaterThan(min(g_dirStep, vec3(0.0)) * (g_dataPos - in_texMin[0]), vec3(0.0))))
        {
            break;
        }

        if((g_fragColor.a > g_opacityThreshold) ||
            g_currentT >= g_terminatePointMax)
        {
            break;
        }
        ++g_currentT;
    }
    return g_fragColor;
}

void finalizeRayCast()
{
    g_fragColor.r = g_fragColor.r * in_scale + in_bias * g_fragColor.a;
    g_fragColor.g = g_fragColor.g * in_scale + in_bias * g_fragColor.a;
    g_fragColor.b = g_fragColor.b * in_scale + in_bias * g_fragColor.a;
    FragColor = g_fragColor;
}

void main()
{
    initializeRayCast();
    castRay(-1.0, -1.0);
    finalizeRayCast();
}











