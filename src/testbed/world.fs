#version 330 core
out vec4 FragColor;

in VS_OUT {
    in vec3 WorldPos; 
    in vec3 Normal;
    in vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;

struct DirectLights {
    int  count;
    mat4 matrics[2];
    vec3 direction[2]; 
    vec3 color[2]; 

    sampler2D shadowMap0;
    sampler2D shadowMap1;
};  
uniform DirectLights dirLights;

struct PointLights {
    bool debug;
    int  count;
    float farPlane;
    vec3 position[2]; 
    vec3 color[2]; 

    samplerCube cubeMap0;
    samplerCube cubeMap1;
};  
uniform PointLights ptLights;

// render control flags
uniform int lightId;        // draw specified point light color
uniform int lightingModel;  // light controls

// constant
float Weight_Ambient = 0.05f;
float Weight_Diffuse = 0.85f;
float Weight_Specular = 0.1f;
float Shininess = 64.f;
float Attenuate_Constant = 1.f;
float Attenuate_Linear = 0.09f;
float Attenuate_Quadratic = 0.032;

float PtShadowCalculation(int id)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fs_in.WorldPos - ptLights.position[id];
    float closestDepth = (id == 0) ?  
                         texture(ptLights.cubeMap0, fragToLight).r : 
                         texture(ptLights.cubeMap1, fragToLight).r ; 

    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= ptLights.farPlane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
  
    float bias = 0.05f;
    float shadow = (currentDepth-bias > closestDepth) ? 1.0 : 0.0 ;        
    return shadow;
}

vec3 PtShadowDebug(int id)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fs_in.WorldPos - ptLights.position[id];
    float closestDepth = (id == 0) ?  
                         texture(ptLights.cubeMap0, fragToLight).r : 
                         texture(ptLights.cubeMap1, fragToLight).r ; 
    return vec3(closestDepth);
}

vec3 PointLighting(vec3 norm, vec3 viewDir, int id) 
{
    //debug code
    if (ptLights.debug) {
        return PtShadowDebug(id);
    }

    vec3 sumLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);

    vec3 lightDir = normalize(ptLights.position[id] - fs_in.WorldPos); 
    float cosTheta = max(dot(norm, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    float distance = length(ptLights.position[id] - fs_in.WorldPos);
    float attenuation = 1.0 / (Attenuate_Constant + Attenuate_Linear*distance +  Attenuate_Quadratic*(distance*distance));    
    
    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex) * attenuation;
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex) * attenuation;
    specular = Weight_Specular * specReflect * vec3(spacularTex) * attenuation;

    // check shadow
    float shadow = 0;//PtShadowCalculation(id);

    sumLight = (ambient + (1.f-shadow)*(diffuse + specular))*ptLights.color[id];
    return sumLight;
}

float DirShadowCalculation(vec4 worldPosLightSpace, vec3 lightDir, int id) 
{
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range
    float closestDepth = (id == 0) ?  
                         texture(dirLights.shadowMap0, projCoords.xy).r : 
                         texture(dirLights.shadowMap1, projCoords.xy).r ; 
    float currentDepth = projCoords.z;

    if(projCoords.z > 1.0) {
        return 0.f;
    }
   
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    float shadow = (currentDepth-bias > closestDepth) ? 1.f : 0.0;

    //shadow = refineShadow(shadow, projCoords, bias, i);
    return shadow;
}

vec3 DirectionLighting(vec3 norm, vec3 viewDir, int id) 
{
    vec3 sumLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    float shadow = 0.f;

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
  
    vec3 lightDir = normalize(dirLights.direction[id]);
    float cosTheta = max(dot(norm, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex);
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex);
    specular = Weight_Specular * specReflect * vec3(spacularTex);

    // check shadow
    vec4 wPosLightSpace = dirLights.matrics[id] * vec4(fs_in.WorldPos, 1.0); 
    shadow = DirShadowCalculation(wPosLightSpace, lightDir, id);

    sumLight = (ambient + (1.f-shadow)*(diffuse + specular))*dirLights.color[id];
    return sumLight;
}

vec3 BlinnPhong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec3 ptSumLight = vec3(0.f);
    vec3 dirSumLight = vec3(0.f);
    for (int i=0; i<ptLights.count; i++) {
        ptSumLight += PointLighting(norm, viewDir, i);
    }
    for (int i=0; i<dirLights.count; i++) {
        dirSumLight += DirectionLighting(norm, viewDir, i);
    }
    return ptSumLight + dirSumLight;
}

void main()
{    
    if (lightId >= 0) {
        // draw point light sphere only
        FragColor = vec4(ptLights.color[lightId], 1.0);
        return;
    }

    vec3 result = vec3(0.f);
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.WorldPos);

    if (lightingModel == 0) {
        result = BlinnPhong_Lighting(norm, viewDir);
    } else if (lightingModel == 1) {
        for (int i=0; i<ptLights.count; i++) {
            result += PointLighting(norm, viewDir, i);
        }
    } else if (lightingModel == 2) {
        for (int i=0; i<dirLights.count; i++) {
            result += DirectionLighting(norm, viewDir, i);
        }
    }
    FragColor = vec4(result, 1.0);
}