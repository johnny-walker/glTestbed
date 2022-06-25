#version 330 core
out vec4 FragColor;

in VS_OUT {
    in vec3 WorldPos; 
    in vec3 Normal;
    in vec2 TexCoords;
    in vec4 WorldPosLightSpace;
} fs_in;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D shadowMap;

uniform vec3 ViewPos; 
uniform vec3 PointLightPos; 
uniform vec3 PointLightColor; 

uniform vec3 DirLightDir; 
uniform vec3 DirLightColor; 

// render control flags
uniform int RenderMode;     
uniform int LightingModel;  

// constant
float abmient_weight = 0.05f;
float diffuse_weight = 0.85f;
float specular_weight = 0.1f;
float shininess = 64.f;

float ShadowCalculation(vec4 worldPosLightSpace, vec3 lightDir) 
{
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
   
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

vec3 PointLighting(vec3 norm, vec3 viewDir) 
{
    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);

    // diffuse shading
    vec3 lightDir = normalize(PointLightPos - fs_in.WorldPos); 
    float cosTheta = max(dot(norm, lightDir), 0.0);
    
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    
    // point light combined results
    vec3 ambient = abmient_weight * vec3(diffuseTex);
    vec3 diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
    vec3 specular = specular_weight * spec * vec3(spacularTex);
    
    return (ambient + diffuse + specular) * PointLightColor;
}

vec3 DirectionLighting(vec3 norm, vec3 viewDir) 
{
    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
  
    // diffuse shading
    vec3 lightDir = normalize(DirLightDir);
    float cosTheta = max(dot(norm, lightDir), 0.0);

    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);

    // direction light combined results
    vec3 ambient = abmient_weight * vec3(diffuseTex);
    vec3 diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
    vec3 specular = specular_weight * spec * vec3(spacularTex);
    
    float shadow = ShadowCalculation(fs_in.WorldPosLightSpace, lightDir);
    return  (ambient + (1.f-shadow)*(diffuse + specular)) * DirLightColor;
}

vec3 BlinnPhong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec3 ptLight = PointLighting(norm, viewDir);
    vec3 dirLight = DirectionLighting(norm, viewDir);
    return ptLight + dirLight;
}

void main()
{    
    if (RenderMode == 1) {
        // draw point light sphere only
        FragColor = vec4(PointLightColor, 1.0);
        return;
    }

    vec3 result = vec3(0.f);
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(ViewPos - fs_in.WorldPos);

    if (LightingModel == 0) {
        result = BlinnPhong_Lighting(norm, viewDir);
    } else if (LightingModel == 1) {
        result = PointLighting(norm, viewDir);
    } else if (LightingModel == 2) {
        result = DirectionLighting(norm, viewDir);
    }
    FragColor = vec4(result, 1.0);
}