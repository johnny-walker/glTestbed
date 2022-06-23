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

// process keyboard 
void DirLight::processInput(GLFWwindow* glWindow, float delta)
{
    if (glfwGetKey(glWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        updateDirection(delta, 0, 0);    //x:left
    } else if (glfwGetKey(glWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        updateDirection(-delta, 0, 0);   //x:right
    } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        updateDirection(0, delta, 0);    //y:up
    } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        updateDirection(0, -delta, 0);   //y:down
    } else if (glfwGetKey(glWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
        updateDirection(0, 0, delta);    //z:ahead
    } else if (glfwGetKey(glWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        updateDirection(0, 0, -delta);   //z:back
    }
}
