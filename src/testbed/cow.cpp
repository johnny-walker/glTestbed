#include "cow.h"

void Cow::init(Shader* pShader, Camera* pCamera)
{
    pCurShader = pShader;
    pCurCamera = pCamera;
    angle = glm::radians(150.f);
    pos = glm::vec3(0.f, 0.25f, 0.f);
}

void Cow::render()
{
    // create view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(pCurCamera->Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.0f);
    glm::mat4 view = pCurCamera->GetViewMatrix();
    pCurShader->setMat4("projection", projection);
    pCurShader->setMat4("view", view);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    model = glm::translate(model, pos);
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

    pCurShader->setMat4("model", model);
    spot->Draw(*pCurShader);
}

void Cow::updateAngle(float delta) {
    angle += delta;
}

void Cow::updatePos(float deltaX, float deltaY, float deltaZ) {
    pos.x += deltaX;
    pos.y += deltaY;
    pos.z += deltaZ;
}
