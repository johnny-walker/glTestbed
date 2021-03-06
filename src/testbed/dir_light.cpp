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


void DirLight::initShadowMapTexture()
{
    if (depthMapFBO == 0) {
        glGenFramebuffers(1, &depthMapFBO);
        glGenTextures(1, &depthMap);

        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, scrWidth, scrHeight, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    //std::cout <<"ID: "<<identifier<<" mapFBO : "<<depthMapFBO<<" map : "<<depthMap<<std::endl;
}

glm::mat4 DirLight::createMatrix(float nearPlane, float farPlane, bool ortho)
{
    projNearPlane = nearPlane;
    projFarPlane = farPlane;
    orthographic = ortho;

    glm::mat4 lightProjection, lightView;
    if (ortho) {
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    }
    else {
        float aspect = (float)scrWidth / (float)scrHeight;
        lightProjection = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
    }
    lightView = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMtrx = lightProjection * lightView;
    return lightSpaceMtrx;
}

glm::mat4 DirLight::getMatrix()
{
    return lightSpaceMtrx;
}

float DirLight::getProjNearPlane()
{
    return projNearPlane;
}

float DirLight::getProjFarPlane()
{
    return projFarPlane;
}

bool DirLight::isOrthoProjection() {
    return orthographic;
}

unsigned int DirLight::getShadowMap()
{
    return depthMap;
}
unsigned int DirLight::getShadowMapFBO()
{
    return depthMapFBO;
}



