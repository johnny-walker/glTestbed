#include "model_base.h"
BaseModel::BaseModel(int width, int height, string const& path, bool gamma):
BaseObject(width, height)
{
	stbi_set_flip_vertically_on_load(false);
	pModel = new Model(path, gamma);
}

BaseModel::~BaseModel()
{
	delete pModel;
}

void BaseModel::init(Shader* pShader, Camera* pCamera)
{
    BaseObject::init(pShader, pCamera);
}

void BaseModel::render()
{
    BaseObject::render();

    pModel->Draw(*pCurShader);
}

