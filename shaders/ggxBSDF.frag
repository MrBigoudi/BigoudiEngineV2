#version 450

// input world space position
layout(location = 0) in vec3 fWorldPos;
// input view space position
layout(location = 1) in vec3 fViewPos;
// input color
layout(location = 2) in vec4 fCol;
// input normal in the world space
layout(location = 3) in vec3 fWorldNorm;
// input normal in the view space
layout(location = 4) in vec3 fViewNorm;
// input texture coords
layout(location = 5) in vec2 fTex;

// output fragment color
layout(location = 0) out vec4 outColor;

// maximum number of point lights
const int MAX_NB_POINT_LIGHTS = 2<<14;
// maximum number of directional lights
const int MAX_NB_DIRECTIONAL_LIGHTS = 2<<10;
// maximum number of materials
const int MAX_NB_MATERIALS = 10;

// pi constant
const float PI = 3.14159265358979323846;

// epsilon to avoid zero division
const float EPSILON = 1e-5;

/**
 * A structure representing a material
 * @see be::Material
*/
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

/**
 * A structure representing a point light
*/
struct PointLight{
    vec4 _Position;
    vec4 _Color;
    float _Intensity;
};

/**
 * A structure representing a directional light
*/
struct DirectionalLight{
    vec4 _Direction;
    vec4 _Color;
    float _Intensity;
};

/**
 * The camera ubo
*/
layout(set = 0, binding = 0) uniform CameraUbo{
    mat4 _View;
    mat4 _Proj;
} cameraUbo;

/**
 * The lights ubo
*/
layout(set = 1, binding = 0, std140) uniform LightsUbo{
    uint _NbPointLights;
    PointLight _PointLights[MAX_NB_POINT_LIGHTS];
    uint _NbDirectionalLights;
    DirectionalLight _DirectionalLights[MAX_NB_DIRECTIONAL_LIGHTS];
} lightsUbo;

/**
 * The object material ubo
*/
layout(set = 2, binding = 0, std140) uniform MaterialUbo{
    Material _Materials[MAX_NB_MATERIALS];
} materialUbo;

layout(push_constant, std430) uniform Push{
    mat4 _Model;
    uint _MaterialId;
}push;



struct Input{
    vec3 _BaseColor;
    Material _Material;
    vec3 _Win;
    vec3 _Wout;
    vec3 _H;
    vec3 _ShadingNormal;
    vec3 _Tangent;
    vec3 _Bitangent;
    vec3 _GeometricNormal;
    float _Intensity;
};

float sqr(float a){
    return a*a;
}

float cube(float a){
    return a*a*a;
}


// Diffuse
// f base diffuse
float getFD90(float roughness, vec3 h, vec3 wout){
    float hw = abs(dot(h,wout));
    return 0.5f + 2.f*roughness*hw*hw;
}
float getFD(float fd90, vec3 n, vec3 w){
    float nw = abs(dot(n,w));
    return 1.f + (fd90 - 1.f)*(1.f - pow(nw, 5));
}
vec3 getBaseDiffuse(vec3 baseColor, float roughness, vec3 h, vec3 n, vec3 win, vec3 wout){
    float fd90 = getFD90(roughness, h, wout);
    float fdwin = getFD(fd90, n, win);
    float fdwout = getFD(fd90, n, wout);
    float factor = abs(dot(n,wout));
    vec3 color = baseColor / PI;
    return color * factor * fdwin * fdwout;
}
// f subsurface diffuse
float getFSS90(float roughness, vec3 h, vec3 wout){
    float hwout = abs(dot(h, wout));
    return roughness * hwout * hwout;
}
float getFSS(float fss90, vec3 n, vec3 w){
    float nw = abs(dot(n, w));
    return 1.f + (fss90 - 1.f)*(1.f-pow(nw, 5));
}
vec3 getSubsurfaceDiffuse(vec3 baseColor, float roughness, vec3 h, vec3 n, vec3 win, vec3 wout){
    float fss90 = getFSS90(roughness, h, wout);
    float fsswin = getFSS(fss90, n, win);
    float fsswout = getFSS(fss90, n, wout);
    float nwin = abs(dot(n, win));
    float nwout = abs(dot(n, wout));
    float factor = 1.f / (nwin + nwout) - 0.5f;
    vec3 color = 1.25f*baseColor / PI;
    return color * (fsswin * fsswout * factor + 0.5f) * nwout;
}
// f diffuse
vec3 getDiffuse(Input i){
    vec3 baseDiffuse = getBaseDiffuse(i._BaseColor, i._Material._Roughness, i._H, i._ShadingNormal, i._Win, i._Wout);
    vec3 subsurfaceDiffuse = getSubsurfaceDiffuse(i._BaseColor, i._Material._Roughness, i._H, i._ShadingNormal, i._Win, i._Wout);
    return (1.f - i._Material._Subsurface) * baseDiffuse + i._Material._Subsurface * subsurfaceDiffuse;
}


// Clearcoat
// Fc
float getR0eta(float eta){
    float num = (eta - 1)*(eta - 1);
    float den = (eta + 1)*(eta + 1);
    return num / den;
}
vec3 getFclearcoat(vec3 h, vec3 wout){
    float r0 = getR0eta(1.5f);
    float hw = pow(1.f - abs(dot(h, wout)), 5);
    return r0*vec3(1.f, 1.f, 1.f) + (1.f - r0)*hw*vec3(1.f, 1.f, 1.f);
}
// Dc
float getAlphag(float clearcoatGloss){
    return (1.f - clearcoatGloss) * 0.1f + clearcoatGloss * 0.001;
}
// TODO:
// need surface tangents to use the BSDF 
float getDclearcoat(float clearcoatGloss, vec3 h, vec3 n){
    // vec3 hl = normalize(h.x*localTangent + h.y*localBiTangent + h.z*localNormal);
    float hn = clamp(dot(n, h), 0.f, 1.f);
    float ag = getAlphag(clearcoatGloss) * getAlphag(clearcoatGloss);
    float num = ag - 1.f;
    float den = PI * log(ag) * (1.f + (ag - 1.f) * (hn * hn));
    return num / den;
}
// Gc
// TODO:
// need surface tangents to use the BSDF 
// float getLambdaC(vec3 w, vec3 localNormal, vec3 localTangent, vec3 localBiTangent){
float getLambdaC(vec3 w, vec3 n){
    // vec3 wl = normalize(w.x*localTangent + w.y*localBiTangent + w.z*localNormal);
    // float wx = (wl.x * 0.25f) * (wl.x * 0.25f);
    // float wy = (wl.y * 0.25f) * (wl.y * 0.25f);
    // float wz = wl.z * wl.z;
    float wn = clamp(dot(n, w), 0.f, 1.f);
    float factor = sqrt(1.f + wn);
    return (factor - 1.f) / 2.f;
}
float getGclearcoatW(vec3 w, vec3 n){
    // return 1.f / (1.f + getLambdaC(w, localNormal, localTangent, localBiTangent));
    return 1.f / (1.f + getLambdaC(w, n));
}
float getGclearcoat(vec3 win, vec3 wout, vec3 n){
    float gwin = getGclearcoatW(win, n);
    float gwout = getGclearcoatW(wout, n);
    return  gwin*gwout;
}
// f clearcoat
vec3 getClearcoat(Input i){
    vec3 Fc = getFclearcoat(i._H, i._Wout);
    float Dc = getDclearcoat(i._Material._ClearcoatGloss, i._H, i._ShadingNormal);
    float Gc = getGclearcoat(i._Win, i._Wout, i._ShadingNormal);
    return (Fc * Dc * Gc) / (4.f*abs(dot(i._ShadingNormal, i._Win)));
}

// Sheen
vec3 getCtint(vec3 baseColor, float intensity){
    if(abs(intensity) > EPSILON){
        return baseColor / intensity;
    }
    return vec3(1.f, 1.f, 1.f);
}
vec3 getCsheen(vec3 baseColor, float intensity, float sheenTint){
    return (1.f - sheenTint) * vec3(1.f, 1.f, 1.f) + sheenTint * getCtint(baseColor, intensity);
}
vec3 getSheen(Input i){
    vec3 csheen = getCsheen(i._BaseColor, i._Intensity, i._Material._SheenTint);
    float hw = pow(1.f - abs(dot(i._H, i._Wout)), 5);
    return csheen * hw * abs(dot(i._ShadingNormal, i._Wout));
}


// Metals
// Fm
vec3 getFmetal(vec3 baseColor, vec3 h, vec3 wout, float specularTint, float intensity, float specular, float metallic){
    vec3 ks = (1.f - specularTint)*vec3(1.f, 1.f, 1.f) + specularTint*getCtint(baseColor, intensity);
    // TODO: use eta instead of 1.5f
    vec3 c0 = specular*getR0eta(1.5f)*(1.f - metallic)*ks + metallic*baseColor;
    float hwout = abs(dot(h, wout));
    return c0 + (vec3(1.f, 1.f, 1.f) - c0)*(1.f-pow(hwout, 5));
}
// Dm
float getAspect(float anisotropic){
    return sqrt(1.f - 0.9f*anisotropic);
}
float getAlphaX(float roughness, float anisotropic){
    float alphaMin = 1e-4;
    float aspect = getAspect(anisotropic);
    return max(alphaMin, roughness*roughness*aspect);
}
float getAlphaY(float roughness, float anisotropic){
    float alphaMin = 1e-4;
    float aspect = getAspect(anisotropic);
    return max(alphaMin, roughness*roughness/aspect);
}
// TODO:
// need surface tangents to use the BSDF 
float getDmetal(float roughness, float anisotropic, vec3 h, vec3 n){
    float hn = clamp(dot(n, h), 0.f, 1.f);
    float r = roughness;
    float alphaX = getAlphaX(r, anisotropic);
    float alphaY = getAlphaY(r, anisotropic);
    float a2 = alphaX * alphaY;

    float den = PI * sqr(1+(a2-1)*sqr(hn)) + EPSILON;
    return a2 / den;
}
// Gm
float getGmetalSmith(vec3 w, vec3 n, float alpha){
    float dotNW = clamp(dot(n, w), 0.f, 1.f);
    float a2 = sqr(alpha);
    float den = (dotNW + (sqrt(a2 + (1-a2) * sqr(dotNW)))) + EPSILON;
    return (2.f*dotNW) / den;
}
// TODO:
// need surface tangents to use the BSDF 
float getLambdaM(float roughness, float anisotropic, vec3 w, vec3 n){
    float wn = clamp(dot(n, w), 0.f, 1.f);

    float alphaX = getAlphaX(roughness, anisotropic);
    float alphaY = getAlphaY(roughness, anisotropic);
    float a2 = clamp(alphaX*alphaY, 0.f, 1.f);

    float den = wn + (sqrt(a2 + (1-a2) * sqr(wn)));
    return (2.f*wn) / den;
}
float getGmetalW(float roughness, float anisotropic, vec3 w, vec3 n){
    // return 1.f / (1.f + getLambdaM(roughness, anisotropic, w, localNormal, localTangent, localBiTangent));
    return 1.f / (1.f + getLambdaM(roughness, anisotropic, w, n));
}
// float getGmetal(float roughness, float anisotropic, vec3 win, vec3 wout, vec3 localNormal, vec3 localTangent, vec3 localBiTangent){
float getGmetal(float roughness, float anisotropic, vec3 win, vec3 wout, vec3 n){
    // float gwin = getGmetalW(roughness, anisotropic, win, localNormal, localTangent, localBiTangent);
    // float gwout = getGmetalW(roughness, anisotropic, wout, localNormal, localTangent, localBiTangent);
    float gwin = getGmetalSmith(win, n, roughness);
    float gwout = getGmetalSmith(wout, n, roughness);
    // float gwin = getGmetalW(roughness, anisotropic, win, n);
    // float gwout = getGmetalW(roughness, anisotropic, wout, n);
    return  gwin*gwout;
}
// f metal
vec3 getMetal(Input i){
    vec3 Fm = getFmetal(i._BaseColor, i._H, i._Wout, i._Material._SpecularTint, i._Intensity, i._Material._Specular, i._Material._Metallic);
    // float Dm = getDmetal(i._H, i._Material._Roughness, i._Material._Anisotropic, i._ShadingNormal, i._Tangent, i._Bitangent);
    // float Gm = getGmetal(i._Material._Roughness, i._Material._Anisotropic, i._Win, i._Wout, i._ShadingNormal, i._Tangent, i._Bitangent);
    float Dm = getDmetal(i._Material._Roughness, i._Material._Anisotropic, i._H, i._ShadingNormal);
    float Gm = getGmetal(i._Material._Roughness, i._Material._Anisotropic, i._Win, i._Wout, i._ShadingNormal);
    return (Fm * Dm * Gm) / (4.f*abs(dot(i._ShadingNormal, i._Win)));
}


// helpers
vec3 getWo(PointLight light, vec3 objViewPos){
    vec3 lightViewPos = vec3(cameraUbo._View * light._Position);
    return normalize(lightViewPos - objViewPos);
}
vec3 getWo(DirectionalLight light){
    return normalize(-light._Direction.xyz);
}
vec3 getWi(vec3 objViewPos){
    return normalize(-objViewPos);
}
vec3 getWh(vec3 wi, vec3 wo){
    return normalize(wi+wo);
}
vec3 getAttenuation(PointLight l, vec3 lightViewPos, vec3 fragViewPos) {
	float d = distance(lightViewPos, fragViewPos);
    float den = d*d + EPSILON;
	return l._Intensity * l._Color.xyz / den;
}
vec3 getAttenuation(DirectionalLight l) {
	return l._Intensity * l._Color.xyz;
}


// main
vec3 getFdisney(Input i){
    vec3 diffuse = (1.f - i._Material._Specular) * (1.f - i._Material._Metallic) * getDiffuse(i);
    vec3 metal = (1.f - i._Material._Specular * (1.f - i._Material._Metallic)) * getMetal(i);
    vec3 clearcoat = 0.25f * i._Material._Clearcoat * getClearcoat(i);
    vec3 sheen = (1.f - i._Material._Metallic) * i._Material._Sheen * getSheen(i);

    // float factor = 1.f;
    // if(dot(i._Win, i._ShadingNormal) <= 0){
    //     factor = 0.f;
    // }
    float factor = max(0, dot(i._Wout, i._ShadingNormal));
    return factor * (diffuse + metal + clearcoat + sheen);
}

Input initInput(){
    Input i;
    i._ShadingNormal = normalize(fViewNorm);
    i._Win = getWi(fViewPos);
    i._Material = materialUbo._Materials[push._MaterialId];
    i._BaseColor = fCol.xyz;
    i._GeometricNormal = normalize(fWorldNorm);//TODO: change that
    i._Tangent = vec3(1.f, 0.f, 0.f);//TODO: change that
    i._Bitangent = cross(i._Tangent, i._ShadingNormal);
    return i;
}


void main(){
    vec3 finalColor = vec3(0.f);
    Input i = initInput();

    // point lights
    for(int k=0; k<min(MAX_NB_POINT_LIGHTS, lightsUbo._NbPointLights); k++){
        PointLight curPointLight = lightsUbo._PointLights[k];
        i._Wout = getWo(curPointLight, fViewPos);
        i._H = getWh(i._Win, i._Wout);
        i._Intensity = curPointLight._Intensity;
        vec3 li = getAttenuation(
            curPointLight, 
            vec3(cameraUbo._View * curPointLight._Position),
            fViewPos
        );
        finalColor += getFdisney(i) * li;
    }

    // directional lights
    for(int k=0; k<min(MAX_NB_DIRECTIONAL_LIGHTS, lightsUbo._NbDirectionalLights); k++){
        DirectionalLight curDirectionalLight = lightsUbo._DirectionalLights[k];
        i._Win = getWo(curDirectionalLight);
        i._H = getWh(i._Win, i._Wout);
        i._Intensity = curDirectionalLight._Intensity;
        vec3 li = getAttenuation(curDirectionalLight);
        finalColor += li * getFdisney(i);
    }

    outColor = vec4(finalColor, 1.f);
}
