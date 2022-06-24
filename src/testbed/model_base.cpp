#include "model_base.h"

void BaseModel::init(Shader* pShader, Camera* pCamera)
{
    BaseObject::init(pShader, pCamera);
}

void BaseModel::render()
{
    BaseObject::render();

    pModel->Draw(*pCurShader);
}

