#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos; 
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
 
void main()
{
    FragPos =aPos;
    vec4 v4norm =  model * vec4(aNormal, 1.0);
    Normal = v4norm.rgb;
    TexCoords =  aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}