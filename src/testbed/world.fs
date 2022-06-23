#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal; 
in vec2 TexCoords;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

uniform vec3 ViewPos; 

uniform vec3 PointLightPos; 
uniform vec3 PointLightColor; 

uniform vec3 DirLightDir; 
uniform vec3 DirLightColor; 

// render control flags
uniform int RenderMode;     //0:draw scene, 1:draw point light
uniform int LightingModel;  //0:Phong, 1:Blinn Phone, 2:PBR

// constant
float abmient_weight = 0.05f;
float diffuse_weight = 0.8f;
float specular_weight = 0.15f;
float shininess = 32.f;

vec3 Phong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec4 diffuseTex = texture(texture_diffuse, TexCoords);
    vec4 spacularTex = texture(texture_specular, TexCoords);
    
    // point light
    // diffuse shading
    vec3 lightDir = normalize(PointLightPos - FragPos); 
    float cosTheta = max(dot(norm, lightDir), 0.0);
   
    // specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // point light combined results
    vec3 ambient = abmient_weight * vec3(diffuseTex);
    vec3 diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
    vec3 specular = specular_weight * spec * vec3(spacularTex);
    
    vec3 ptLight = (ambient + diffuse + specular) * PointLightColor;

    // direction light
    // diffuse shading
    lightDir = normalize(DirLightDir);
    cosTheta = max(dot(norm, lightDir), 0.0);

    // specular shading
    reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // direction light combined results
    ambient = abmient_weight * vec3(diffuseTex);
    diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
    specular = specular_weight * spec * vec3(spacularTex);
    
    vec3 dirLight = (ambient + diffuse + specular) * DirLightColor;

    return ptLight + dirLight;
}

vec3 BlinnPhong_Lighting(vec3 norm, vec3 viewDir) 
{
    // todo
    return vec3(0.f);
}

void main()
{    
    if (RenderMode == 1) {
        // draw point light only
        FragColor = vec4(PointLightColor, 1.0);
        return;
    }

    vec3 result = vec3(0.f);
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(ViewPos - FragPos);

    if (LightingModel == 0) {
        // draw point light only
        result = Phong_Lighting(norm, viewDir);
    }
    FragColor = vec4(result, 1.0);
}