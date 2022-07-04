#include "dir_light.h"
DirLight::DirLight(int id, int width, int height) :
Light(id, width, height) 
{
}

DirLight::~DirLight() {
}

void DirLight::init(Shader* pShader, Camera* pCamera)
{
    Light::init(pShader, pCamera);
    initShadowMapTexture();
}

void DirLight::render()
{
    Light::render();
}

