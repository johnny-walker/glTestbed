#include "light.h"

void Light::init(Shader* pShader, Camera* pCamera)
{
    pCurShader = pShader;
    pCurCamera = pCamera;

    // init colors
    colors.push_back(glm::vec3(1.f, 1.f, 1.f));             //white (255, 255, 255)
    colors.push_back(glm::vec3(1.f, 0.f, 0.f));             //red (255, 0, 0)
    colors.push_back(glm::vec3(1.f, 165.f / 255.f, 0.f));   //orange (255, 165, 0)
    colors.push_back(glm::vec3(1.f, 1.f, 0.f));             //yellow (255,255,0)
    colors.push_back(glm::vec3(0.f, 1.0f, 0.f));            //green (0,128,0)
    colors.push_back(glm::vec3(0.f, 1.f, 1.f));             //cyan (0,255,255)
    colors.push_back(glm::vec3(0.f, 0.f, 1.f));             //blue (0,0,255)
    colors.push_back(glm::vec3(0.5f, 0.f, 0.5f));           //purple (128,0,128)
    colors.push_back(glm::vec3(1.f, 0.f, 1.f));             //magenta (255, 0, 255)
}

void Light::render()
{
}

void Light::setPrimaryColor(int index)
{
    if (index >=0 && index <= 8) 
        lightColor = colors[index];
}

void Light::setColor(glm::vec3 color)
{
    lightColor = color;
}

void Light::adjustStrength(float value)
{
    strength += value;
    strength = min(max(strength, 0.f), 1.f);  
}
