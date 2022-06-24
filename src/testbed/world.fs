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
uniform vec3 PointLightPos; 
uniform vec3 PointLightColor; 

uniform vec3 DirLightDir; 
uniform vec3 DirLightColor; 

// render control flags
uniform int RenderMode;     
uniform int LightingModel;  //0:Phong, 1:Blinn Phone, 2:PBR

// constant
float abmient_weight = 0.05f;
float diffuse_weight = 0.8f;
float specular_weight = 0.15f;
float shininess = 32.f;

vec3 Phong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
    
    // point light
    // diffuse shading
    vec3 lightDir = normalize(PointLightPos - fs_in.WorldPos); 
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

vec3 Blinn_Phong_Lighting(vec3 norm, vec3 viewDir) 
{
    vec4 diffuseTex = texture(texture_diffuse, fs_in.TexCoords);
    vec4 spacularTex = texture(texture_specular, fs_in.TexCoords);
    
    // point light
    // diffuse shading
    vec3 lightDir = normalize(PointLightPos - fs_in.WorldPos); 
    float cosTheta = max(dot(norm, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // point light combined results
    vec3 ambient = abmient_weight * vec3(diffuseTex);
    vec3 diffuse = diffuse_weight * cosTheta * vec3(diffuseTex);
    vec3 specular = specular_weight * spec * vec3(spacularTex);
    
    vec3 ptLight = (ambient + diffuse + specular) * PointLightColor;

    return ptLight;
}

void main()
{    
    if (RenderMode == 1) {
        // draw point light only
        FragColor = vec4(PointLightColor, 1.0);
        return;
    }

    vec3 result = vec3(0.f);
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(ViewPos - fs_in.WorldPos);

    if (LightingModel == 0) {
        result = Phong_Lighting(norm, viewDir);
    } else if (LightingModel == 1) {
        result = Blinn_Phong_Lighting(norm, viewDir);
    } else {
        //todo
        result = vec3(1.f);
    }
    FragColor = vec4(result, 1.0);
}