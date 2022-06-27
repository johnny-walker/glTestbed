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

// point lights
uniform int  PointLightDrawId;
uniform int  PointLightCount;
uniform vec3 PointLightPos[5]; 
uniform vec3 PointLightColor[5]; 

// direction lights
uniform int  DirLightCount;
uniform vec3 DirLightDir[5]; 
uniform vec3 DirLightColor[5]; 

// render control flags
uniform int RenderMode;     
uniform int LightingModel;  

// constant
float abmient_weight = 0.05f;
float diffuse_weight = 0.85f;
float specular_weight = 0.1f;
float shininess = 64.f;

//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
float refineShadow(float shadow, vec3 projCoords, float bias) 
{
    vec2 poissonDisk[4] = vec2[](vec2( -0.94201624, -0.39906216 ),
                                 vec2( 0.94558609, -0.76890725 ),
                                 vec2( -0.094184101, -0.92938870 ),
                                 vec2( 0.34495938, 0.29387760 ) );
    for (int i=0; i<4; i++){
        float neighborDepth = texture(shadowMap, projCoords.xy + poissonDisk[i]/700.f ).z;
        if ( neighborDepth < projCoords.z - bias ) {
            shadow -= 0.1f;
        }
    }
    return max(min(shadow, 1.f), 0.f);
}

float ShadowCalculation(vec4 worldPosLightSpace, vec3 lightDir) 
{
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    if(projCoords.z > 1.0) {
        return 0.f;
    }
   
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.f : 0.0;

    shadow = refineShadow(shadow, projCoords, bias);
    return shadow;
}

vec3 PointLighting(vec3 norm, vec3 viewDir) 
{
    vec3 ptLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);

    for (int i=0; i<PointLightCount; i++) {
        // diffuse shading
        vec3 lightDir = normalize(PointLightPos[i] - fs_in.WorldPos); 
        float cosTheta = max(dot(norm, lightDir), 0.0);
    
        // specular shading
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    
        // point light combined results
        ambient = abmient_weight * vec3(diffuseTex);
        diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
        specular = specular_weight * spec * vec3(spacularTex);
        ptLight += (ambient + diffuse + specular)*PointLightColor[i];
    }
    
    return ptLight;
}

vec3 DirectionLighting(vec3 norm, vec3 viewDir) 
{
    vec3 dirLight = vec3(0.f);
    vec3 ambient = vec3(0.f);
    vec3 diffuse = vec3(0.f);
    vec3 specular = vec3(0.f);

    float shadow = 0.f;

    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
  
    for (int i=0; i<1; i++) {
        // diffuse shading
        vec3 lightDir = normalize(DirLightDir[i]);
        float cosTheta = max(dot(norm, lightDir), 0.0);

        // specular shading
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);

        // direction light combined results
        ambient = abmient_weight * vec3(diffuseTex);
        diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
        specular = specular_weight * spec * vec3(spacularTex);

        shadow = ShadowCalculation(fs_in.WorldPosLightSpace, lightDir);
        dirLight += (ambient + (1.f-shadow)*(diffuse + specular))*DirLightColor[i];
    }
    
    return dirLight;
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
        FragColor = vec4(PointLightColor[PointLightDrawId], 1.0);
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