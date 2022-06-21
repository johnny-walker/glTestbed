#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal; 
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos; 
uniform vec3 lightColor; 

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos); 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * FragColor.rgb;
    FragColor = vec4(result, 1.0);
}