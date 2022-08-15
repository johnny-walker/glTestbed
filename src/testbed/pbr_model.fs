#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos; 
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;

// GL_TEXTURE0 + i
// model texture maps
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_orm1;

// flags if using texutre mpas
uniform bool specularMap;
uniform bool normalMap;
uniform bool aoMap;
uniform bool roughnessMap;
uniform bool metallicMap;
uniform bool ormMap;

// GL_TEXTURE6 + i
struct DirectLights {
    int  count;
    mat4 matrics[2];
    vec3 direction[2]; 
    vec3 color[2]; 

    sampler2D shadowMap0;
    sampler2D shadowMap1;
};  
uniform DirectLights dirLights;

// GL_TEXTURE8 + i
struct PointLights {
    int  count;
    mat4 matrics[2];
    vec3 position[2]; 
    vec3 color[2]; 
    float nearPlane;
    float farPlane;

    // cubemap
    samplerCube cubeMap0;
    samplerCube cubeMap1;
};  
uniform PointLights ptLights;

// GL_TEXTURE10 + i
// IBL maps
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;


// render control flags
uniform int lightId;        // draw specified point light color
uniform int lightingModel;  // light controls

const float Shadow_Bias = 0.05f;
const float Weight_Ambient = 0.03f;
const float Attenuate_Constant = 1.f;
const float Attenuate_Linear = 0.09f;
const float Attenuate_Quadratic = 0.032;
const float PI = 3.14159265359f;

vec3 getNormalFromMap()
{
    if (!normalMap) {
        // return fragment normal
        return normalize(fs_in.Normal);
    }

    vec3 tangentNormal = vec3(texture(texture_normal1, fs_in.TexCoords)) * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T   = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 BRDF_Lighting(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 F0, vec3 albedo, 
                   float roughness, float metallic, int i)
{
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = FresnelSchlickRoughness(clamp(dot(H, V), 0.0, 1.0), F0, roughness);
           
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	

    float NdotL = max(dot(N, L), 0.0);        
    return (kD * albedo/PI + specular) * radiance * NdotL;
}

// direction light
float OrthoGrahicDepth(vec3 projCoords, int id)
{
    float closestDepth = (id == 0) ?  
                         texture(dirLights.shadowMap0, projCoords.xy).r : 
                         texture(dirLights.shadowMap1, projCoords.xy).r ; 
    return closestDepth;
}

float DirShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, int id) 
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range
    float currentDepth = projCoords.z;
    if(projCoords.z > 1.0) {
        return 0.f;
    }
    float closestDepth = OrthoGrahicDepth(projCoords, id); 
   
    float bias = max(0.05 * (1.0 - dot(getNormalFromMap(), lightDir)), 0.005);
    float shadow = (currentDepth-bias > closestDepth) ? 1.f : 0.0;

    return shadow;
}

vec3 DirectionLighting(vec3 N, vec3 V, vec3 albedo, vec3 F0, float ao, float roughness, float metallic) 
{
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < dirLights.count; ++i) {
        vec3 L = normalize(dirLights.direction[i]);
        vec3 radiance = dirLights.color[i];

        vec3 iLo = BRDF_Lighting(N, V, L, radiance, F0, albedo, roughness, metallic, i);
        
        // check shadow
        float shadow = 0.f;
        vec4 wPosLightSpace = dirLights.matrics[i] * vec4(fs_in.FragPos, 1.0); 
        shadow = DirShadowCalculation(wPosLightSpace, L, i);

        iLo = (1.f-shadow)*(iLo);
        Lo += iLo;
    }   

    vec3 ambient = vec3(Weight_Ambient) * albedo * ao;
    vec3 color = ambient + Lo;

    // HDR tonemapping & gamma correct
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    return color;
}

// point light
float PointCubeDepth(vec3 fragToLight, int id)
{
    float closestDepth = (id == 0) ?  
                         texture(ptLights.cubeMap0, fragToLight).r : 
                         texture(ptLights.cubeMap1, fragToLight).r ; 
    return closestDepth;
}

float PtCubemapShadowCalculation(int id)
{
    // get vector between fragment position and light position
    vec3 fragToLight = (fs_in.FragPos - ptLights.position[id]);
    float closestDepth = PointCubeDepth(fragToLight, id);
    
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= ptLights.farPlane;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
  
    float shadow = (currentDepth-Shadow_Bias > closestDepth) ? 1.0 : 0.0 ;        
    return shadow;
}

// reflectance equation
vec3 PointLighting(vec3 N, vec3 V, vec3 albedo, vec3 F0, float ao, float roughness, float metallic) 
{
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < ptLights.count; ++i) {
        vec3 L = normalize(ptLights.position[i] - fs_in.FragPos);
        float distance = length(ptLights.position[i] - fs_in.FragPos);
        float denominator = Attenuate_Constant + Attenuate_Linear*distance
                            /* + Attenuate_Quadratic*(distance*distance) */;    
        float attenuation = 1.0 / denominator;
        vec3 radiance = ptLights.color[i] * attenuation;

        vec3 iLo = BRDF_Lighting(N, V, L, radiance, F0, albedo, roughness, metallic, i);
               
        // check shadow
        float shadow = 0.f;
        shadow = PtCubemapShadowCalculation(i);

        iLo =  (1.f-shadow)*(iLo);
        Lo += iLo;
    }   
   
    vec3 ambient = vec3(Weight_Ambient) * albedo * ao;
    vec3 color = ambient + Lo;

    // HDR tonemapping & gamma correct
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    return color;
}

vec4 PBR_Lighting(vec3 N, vec3 V) 
{
    vec3 SumLights  = vec3(0.f);

    // sample AO, Roughness, Metallic
    float ao        = 1.f;
    float roughness = 0.f;
    float metallic  = 0.f;
    float alpha     = 1.f;

    vec4 diffuse = texture(texture_diffuse1, fs_in.TexCoords);
    vec3 albedo  = pow(diffuse.rgb, vec3(2.2));
    alpha        = diffuse.a;

    if (ormMap) {
        vec4 ormTex = texture(texture_orm1, fs_in.TexCoords);
        ao        = ormTex.r;
        roughness = ormTex.g;
        metallic  = ormTex.b;
    } else {
        ao        = aoMap ? texture(texture_ao1, fs_in.TexCoords).r : ao;
        roughness = roughnessMap ? texture(texture_roughness1, fs_in.TexCoords).r : roughness;
        metallic  = metallicMap ? texture(texture_metallic1, fs_in.TexCoords).r : metallic;
    }

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    if (lightingModel == 0 || lightingModel == 1) {
        if (dirLights.count > 0)
            SumLights += DirectionLighting(N, V, albedo, F0, ao, roughness, metallic);
    }
    if (lightingModel == 0 || lightingModel == 2) {
        //if (ptLights.count > 0)
            SumLights += PointLighting(N, V, albedo, F0, ao, roughness, metallic);
    }
    return vec4(SumLights, alpha);
}

void main()
{    
    if (lightId >= 0) {
        // draw point light sphere only
        FragColor = vec4(ptLights.color[lightId], 1.0);
        return;
    }

    vec3 N = getNormalFromMap();
    vec3 V = normalize(viewPos - fs_in.FragPos);

    FragColor = PBR_Lighting(N, V);
}