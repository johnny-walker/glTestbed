#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal; 
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform vec3 PointLightPos; 
uniform vec3 PointLightColor; 

uniform vec3 ParalLightDir; 
uniform vec3 ParalLightColor; 

// render flag
uniform int renderMode; 

void main()
{    
    if (renderMode == 1) {
        // draw point light
        FragColor = vec4(PointLightColor, 1.0);
    } else {
        // draw scene objects
        vec3 norm = normalize(Normal);
        FragColor = texture(texture_diffuse1, TexCoords);

        // ambient light
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * PointLightColor;

        // point light
        vec3 lightDir = normalize(PointLightPos - FragPos); 
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * PointLightColor;

        // parallel light
        vec3 lightDir2 = normalize(ParalLightDir);
        float diff2 = max(dot(norm, lightDir2), 0.0);
        vec3 diffuse2 = diff2 * ParalLightColor;
        
        // accumulate the lights
        diffuse = (0.2* diffuse + 0.8* diffuse2);
        vec3 result = (ambient + diffuse) * vec3(FragColor);
        FragColor = vec4(result, 1.0);
    }
}