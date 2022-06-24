#include "dir_light.h"

void DirLight::init(Shader* pShader, Camera* pCamera)
{
    Light::init(pShader, pCamera);
}

void DirLight::render()
{
    Light::render();

    pCurShader->use();
    pCurShader->setVec3("DirLightDir", direction);
    pCurShader->setVec3("DirLightColor", lightColor * strength);
}

void DirLight::setDirection(glm::vec3 dir)
{
    direction = dir;
}

void DirLight::updateDirection(float deltaX, float deltaY, float deltaZ)
{
    direction += glm::vec3(deltaX, deltaY, deltaZ);
}
