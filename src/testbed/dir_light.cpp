#include "dir_light.h"

void DirLight::init(Shader* pShader, Camera* pCamera)
{
    Light::init(pShader, pCamera);
}

void DirLight::render()
{
    Light::render();
    pCurShader->use();
    pCurShader->setVec3("DirLightDir[" + std::to_string(identifier) + "]", pos);
    pCurShader->setVec3("DirLightColor[" + std::to_string(identifier) + "]", lightColor * strength);
}
