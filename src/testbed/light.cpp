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
    dirty = true;
}

void Light::setColor(glm::vec3 color)
{
    lightColor = color;
    dirty = true;
}

void Light::setStrength(float value) 
{
    strength = value;
    strength = min(max(strength, 0.f), 1.f);
    dirty = true;
}

void Light::adjustStrength(float delta)
{
    strength += delta;
    strength = min(max(strength, 0.f), 1.f);  
    dirty = true;
}

void Light::processLight(GLFWwindow* glWindow)
{
    // adjust primary color
    if (glfwGetKey(glWindow, GLFW_KEY_0) == GLFW_PRESS) {
        setPrimaryColor(0);
    } else if (glfwGetKey(glWindow, GLFW_KEY_1) == GLFW_PRESS) {
        setPrimaryColor(1);
    } else if (glfwGetKey(glWindow, GLFW_KEY_2) == GLFW_PRESS) {
        setPrimaryColor(2);
    } else if (glfwGetKey(glWindow, GLFW_KEY_3) == GLFW_PRESS) {
        setPrimaryColor(3);
    } else if (glfwGetKey(glWindow, GLFW_KEY_4) == GLFW_PRESS) {
        setPrimaryColor(4);
    } else if (glfwGetKey(glWindow, GLFW_KEY_5) == GLFW_PRESS) {
        setPrimaryColor(5);
    } else if (glfwGetKey(glWindow, GLFW_KEY_6) == GLFW_PRESS) {
        setPrimaryColor(6);
    } else if (glfwGetKey(glWindow, GLFW_KEY_7) == GLFW_PRESS) {
        setPrimaryColor(7);
    } else if (glfwGetKey(glWindow, GLFW_KEY_8) == GLFW_PRESS) {
        setPrimaryColor(8);
    }
    // adjust light strength
    if (glfwGetKey(glWindow, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        adjustLight = 1;    //+
    } else if (glfwGetKey(glWindow, GLFW_KEY_MINUS) == GLFW_PRESS) {
        adjustLight = 2;    //-
    } else if (glfwGetKey(glWindow, GLFW_KEY_EQUAL) == GLFW_RELEASE && adjustLight == 1) {
        adjustStrength(0.1f);
        adjustLight = 0;
    } else if (glfwGetKey(glWindow, GLFW_KEY_EQUAL) == GLFW_RELEASE && adjustLight == 2) {
        adjustStrength(-0.1f);
        adjustLight = 0;
    }
    dirty = true;
}
