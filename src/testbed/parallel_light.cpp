#include "parallel_light.h"

void ParallelLight::init(Shader* pShader, Camera* pCamera)
{
    Light::init(pShader, pCamera);
}

void ParallelLight::render()
{
    Light::render();

    pCurShader->use();
    pCurShader->setVec3("ParalLightDir", direction);
    pCurShader->setVec3("ParalLightColor", lightColor * strength);
}

void ParallelLight::setDirection(glm::vec3 dir)
{
    direction = dir;
}

void ParallelLight::updateDirection(float deltaX, float deltaY, float deltaZ)
{
    direction += glm::vec3(deltaX, deltaY, deltaZ);
}

// process keyboard 
void ParallelLight::processInput(GLFWwindow* glWindow, float delta)
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
