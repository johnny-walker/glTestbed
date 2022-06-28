#include "dir_light.h"

void DirLight::init(Shader* pShader, Camera* pCamera)
{
    Light::init(pShader, pCamera);
    initShadowMapTexture();
}

void DirLight::render()
{
    Light::render();
    pCurShader->use();
    pCurShader->setVec3("DirLightDir[" + std::to_string(identifier) + "]", pos);
    pCurShader->setVec3("DirLightColor[" + std::to_string(identifier) + "]", lightColor * strength);
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

glm::mat4 DirLight::createLightSpaceMatrix(float nearPlane, float farPlane)
{
    glm::mat4 lightProjection, lightView;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    lightView = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMtrx = lightProjection * lightView;
    return lightSpaceMtrx;
}

glm::mat4 DirLight::getLightSpaceMatrix()
{
    return lightSpaceMtrx;
}

unsigned int DirLight::getShadowMap()
{
    return depthMap;
}
unsigned int DirLight::getShadowMapFBO()
{
    return depthMapFBO;
}
