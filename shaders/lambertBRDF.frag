#version 450

layout(location = 0) in vec4 fPos;
layout(location = 1) in vec4 fCol;
layout(location = 2) in vec3 fNorm;
layout(location = 3) in vec2 fTex;

layout(location = 0) out vec4 outColor;

const int MAX_NB_POINT_LIGHTS = 10;

struct Material {
    float _Metallic;
    float _Subsurface;
    float _Specular;
    float _Roughness;
    float _SpecularTint;
    float _Anisotropic;
    float _Sheen;
    float _SheenTint;
    float _Clearcoat;
    float _ClearcoatGloss;
};

struct PointLight{
    vec4 _Position;
    vec4 _Color;
    float _Intensity;
};

layout(set = 0, binding = 0) uniform CameraUbo{
    mat4 _View;
    mat4 _Proj;
} cameraUbo;

layout(set = 1, binding = 0) uniform LightsUbo{
    uint _NbPointLights;
    PointLight _PointLights[MAX_NB_POINT_LIGHTS];
} lightsUbo;

layout(set = 2, binding = 0) uniform MaterialUbo{
    Material _ObjMaterial;
} materialUbo;


/**
 * Get the diffuse model for the point light
 * @param light The current point light
 * @param objPos The world pos of the current vertex
 * @param objNorm The norm of the current vertex
 * @param objColor The vertex color
 * @return The color value of the response
*/
vec4 getDiffusePointLight(PointLight light, vec4 objPos, vec3 objNorm, vec4 objColor){
    vec4 lightWorldPos = cameraUbo._Proj * cameraUbo._View * light._Position;
    vec3 lightDir = normalize((lightWorldPos - objPos).xyz);
    float weight = light._Intensity * dot(lightDir, objNorm);
    return max(weight, 0.f) * light._Color * objColor;
}


void main() {
    vec3 finalColor = vec3(0.f);
    for(int i=0; i<lightsUbo._NbPointLights; i++){
        PointLight curPointLight = lightsUbo._PointLights[i];
        finalColor += getDiffusePointLight(curPointLight, fPos, normalize(fNorm), fCol).xyz;
    }
    outColor = vec4(finalColor, 1.f);
    
}