#ifndef MODLE_BASE_H
#define MODLE_BASE_H
#include "object.h"

class BaseModel: public BaseObject
{
public: 
	BaseModel(int width, int height, string const& path, bool gamma = false) :
		BaseObject(width, height)
	{
		stbi_set_flip_vertically_on_load(false);
		pModel = new Model(path, gamma);
	}
	~BaseModel()
	{
		delete pModel;
	}

public:
	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	Model* pModel = nullptr;
};
#endif //MODLE_BASE_H

