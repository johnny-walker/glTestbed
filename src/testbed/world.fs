#version 330 core
out vec4 FragColor;

in VS_OUT {
    in vec3 WorldPos; 
    in vec3 Normal;
    in vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;

uniform vec3 ViewPos;

// direction lights shadow mapping
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform mat4 lightSpaceMatrix0;
uniform mat4 lightSpaceMatrix1;

uniform int  DirLightCount;
uniform vec3 DirLightDir[2]; 
uniform vec3 DirLightColor[2]; 

// point lights shadow mapping
uniform samplerCube cubeMap0;
uniform samplerCube cubeMap1;
uniform float FarPlane;
uniform int  PointLightDrawId;

uniform int  PointLightCount;
uniform vec3 PointLightPos[2]; 
uniform vec3 PointLightColor[2]; 

// render control flags
uniform int RenderMode;     
uniform int LightingModel;  

// constant
float Weight_Ambient = 0.05f;
float Weight_Diffuse = 0.85f;
float Weight_Specular = 0.1f;
float Shininess = 64.f;
float Attenuate_Constant = 1.f;
float Attenuate_Linear = 0.09f;
float Attenuate_Quadratic = 0.032;

//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
float refineShadow(float shadow, vec3 projCoords, float bias, int lightId) 
{
    vec2 poissonDisk[4] = vec2[](vec2( -0.94201624, -0.39906216 ),
                                 vec2( 0.94558609, -0.76890725 ),
                                 vec2( -0.094184101, -0.92938870 ),
                                 vec2( 0.34495938, 0.29387760 ) );

    for (int i=0; i<4; i++){
        float denom = 500.f;
        float neighborDepth = (lightId == 0) ? 
                   texture(shadowMap0, projCoords.xy + poissonDisk[i]/denom ).z :
                   texture(shadowMap1, projCoords.xy + poissonDisk[i]/denom ).z ;

        if ( neighborDepth < projCoords.z - bias ) {
            shadow -= 0.05f;
        }
    }
    return max(min(shadow, 1.f), 0.f);
}

float ShadowCalculation(vec4 worldPosLightSpace, vec3 lightDir, int i) 
{
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range
    float closestDepth = (i == 0) ?  
                         texture(shadowMap0, projCoords.xy).r : 
                         texture(shadowMap1, projCoords.xy).r ; 
    float currentDepth = projCoords.z;

    if(projCoords.z > 1.0) {
        return 0.f;
    }
   
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.f : 0.0;

    //shadow = refineShadow(shadow, projCoords, bias, i);
    return shadow;
}

vec3 PointLighting(vec3 norm, vec3 viewDir, int i) 
{
    vec3 ptLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);

    vec3 lightDir = normalize(PointLightPos[i] - fs_in.WorldPos); 
    float cosTheta = max(dot(norm, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    float distance = length(PointLightPos[i] - fs_in.WorldPos);
    float attenuation = 1.0 / (Attenuate_Constant + Attenuate_Linear*distance +  Attenuate_Quadratic*(distance*distance));    
    
    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex) * attenuation;
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex) * attenuation;
    specular = Weight_Specular * specReflect * vec3(spacularTex) * attenuation;
    
    ptLight = (ambient + diffuse + specular)*PointLightColor[i];
    return ptLight;
}

vec3 DirectionLighting(vec3 norm, vec3 viewDir, int i) 
{
    vec3 dirLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    float shadow = 0.f;

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
  
    vec3 lightDir = normalize(DirLightDir[i]);
    float cosTheta = max(dot(norm, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float specReflect = pow(max(dot(norm, halfwayDir), 0.0), Shininess);

    // weighted sum
    ambient = Weight_Ambient * vec3(diffuseTex);
    diffuse = Weight_Diffuse * cosTheta * vec3(diffuseTex);
    specular = Weight_Specular * specReflect * vec3(spacularTex);

    vec4 wPosLightSpace = (i == 0) ?  
                          lightSpaceMatrix0 * vec4(fs_in.WorldPos, 1.0) : 
                          lightSpaceMatrix1 * vec4(fs_in.WorldPos, 1.0) ; 
    shadow = ShadowCalculation(wPosLightSpace, lightDir, i);
    dirLight = (ambient + (1.f-shadow)*(diffuse + specular))*DirLightColor[i];
    return dirLight;
}

vec3 BlinnPhong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec3 ptLight = vec3(0.f);
    vec3 dirLight = vec3(0.f);
    for (int i=0; i<PointLightCount; i++) {
        ptLight += PointLighting(norm, viewDir, i);
    }
    for (int i=0; i<DirLightCount; i++) {
        dirLight += DirectionLighting(norm, viewDir, i);
    }
    return ptLight + dirLight;
}

void main()
{    
    if (RenderMode == 1) {
        // draw point light sphere only
        FragColor = vec4(PointLightColor[PointLightDrawId], 1.0);
        return;
    }

    vec3 result = vec3(0.f);
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(ViewPos - fs_in.WorldPos);

    if (LightingModel == 0) {
        result = BlinnPhong_Lighting(norm, viewDir);
    } else if (LightingModel == 1) {
        for (int i=0; i<PointLightCount; i++) {
            result += PointLighting(norm, viewDir, i);
        }
    } else if (LightingModel == 2) {
        for (int i=0; i<DirLightCount; i++) {
            result += DirectionLighting(norm, viewDir, i);
        }
    }
    FragColor = vec4(result, 1.0);
}