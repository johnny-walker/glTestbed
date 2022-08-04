#version 330 core
out vec4 FragColor;

in VS_OUT {
    in vec3 FragPos; 
    in vec3 Normal;
    in vec2 TexCoords;
} fs_in;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;


struct DirectLights {
    int  count;
    mat4 matrics[2];
    vec3 direction[2]; 
    vec3 color[2]; 

    sampler2D shadowMap0;
    sampler2D shadowMap1;

    bool debug;
};  
uniform DirectLights dirLights;

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
    
    bool debug;
};  
uniform PointLights ptLights;

// render control flags
uniform int lightId;        // draw specified point light color
uniform int lightingModel;  // light controls

// constant
float Weight_Ambient = 0.05f;
float Weight_Diffuse = 1.0f;//0.85f;
float Weight_Specular = 0.1f;
float Shininess = 64.f;
float Attenuate_Constant = 1.f;
float Attenuate_Linear = 0.09f;
float Attenuate_Quadratic = 0.032;

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
   
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    float shadow = (currentDepth-bias > closestDepth) ? 1.f : 0.0;

    return shadow;
}

vec3 DirShadowDebug(vec4 fragPosLightSpace, int id)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range
    float closestDepth = OrthoGrahicDepth(projCoords, id); 
    return vec3(closestDepth);
}

vec3 DirectionLighting(vec3 norm, vec3 viewDir, int id) 
{
    vec4 wPosLightSpace = dirLights.matrics[id] * vec4(fs_in.FragPos, 1.0); 
 
    //debug code for shadowmap
    if (dirLights.debug) {
        return DirShadowDebug(wPosLightSpace, id);
    }

    vec3 sumLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    float shadow = 0.f;

    vec4 diffuseTex = texture(texture_diffuse1, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular1, fs_in.TexCoords);
  
    vec3 lightDir = normalize(dirLights.direction[id]);
    float cosTheta = max(dot(norm, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex);
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex);
    specular = Weight_Specular * specReflect * vec3(spacularTex);

    // check shadow
    shadow = DirShadowCalculation(wPosLightSpace, lightDir, id);

    sumLight = (ambient + (1.f-shadow)*(diffuse + specular))*dirLights.color[id];
    return sumLight;
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
  
    float bias = 0.05f;
    float shadow = (currentDepth-bias > closestDepth) ? 1.0 : 0.0 ;        
    return shadow;
}

vec3 PtCubemapDebug(int id)
{
    // get vector between fragment position and light position
    vec3 fragToLight = (fs_in.FragPos - ptLights.position[id]);
    float closestDepth = PointCubeDepth(fragToLight, id);
    //closestDepth = length(fragToLight) / ptLights.farPlane;
    return vec3(closestDepth);
}

vec3 PointLighting(vec3 norm, vec3 viewDir, int id) 
{
    //debug code for cubemap
    if (ptLights.debug) {
        return PtCubemapDebug(id);
    }

    vec3 sumLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    vec4 diffuseTex = texture(texture_diffuse1, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular1, fs_in.TexCoords);

    vec3 lightDir = normalize(ptLights.position[id] - fs_in.FragPos);  
    float cosTheta = max(dot(norm, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    float distance = length(ptLights.position[id] - fs_in.FragPos);
    float attenuation = 1.0 / (Attenuate_Constant + Attenuate_Linear*distance +  Attenuate_Quadratic*(distance*distance));    
    
    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex) * attenuation;
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex) * attenuation;
    specular = Weight_Specular * specReflect * vec3(spacularTex) * attenuation;

    // check shadow
    float shadow = PtCubemapShadowCalculation(id);

    sumLight = (ambient + (1.f-shadow)*(diffuse + specular))*ptLights.color[id];
    return sumLight;
}

vec3 BlinnPhong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec3 ptSumLight = vec3(0.f);
    vec3 dirSumLight = vec3(0.f);
    for (int i=0; i<dirLights.count; i++) {
        dirSumLight += DirectionLighting(norm, viewDir, i);
    }
    for (int i=0; i<ptLights.count; i++) {
        ptSumLight += PointLighting(norm, viewDir, i);
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
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

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