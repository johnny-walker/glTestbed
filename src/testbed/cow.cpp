#include "cow.h"

void Cow::init(Shader* pShader, Camera* pCamera)
{
    pCurShader = pShader;
    pCurCamera = pCamera;
}

void Cow::render()
{
    pCurShader->use();

    // create mvp 
    glm::mat4 projection = glm::perspective(glm::radians(pCurCamera->Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.0f);
    glm::mat4 view = pCurCamera->GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    pCurShader->setMat4("projection", projection);
    pCurShader->setMat4("view", view);

    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    model = glm::translate(model, pos);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    pCurShader->setMat4("model", model);

    spot->Draw(*pCurShader);
}

void Cow::processInput(GLFWwindow* glWindow, float delta)
{
    if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
        updateAngle(delta);        //counter clockwise
    } else if (glfwGetKey(glWindow, GLFW_KEY_T) == GLFW_PRESS) {
        updateAngle(-delta);       //clockwise
    } else if (glfwGetKey(glWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        updatePos(delta, 0, 0);    //x:left
    } else if (glfwGetKey(glWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        updatePos(-delta, 0, 0);   //x:right
    } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        updatePos(0, delta, 0);    //y:up
    } else if (glfwGetKey(glWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        updatePos(0, -delta, 0);   //y:down
    } else if (glfwGetKey(glWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
        updatePos(0, 0, delta);    //z:ahead
    } else if (glfwGetKey(glWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        updatePos(0, 0, -delta);   //z:back
    }
}


