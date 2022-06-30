#ifndef MODLE_BASE_H
#define MODLE_BASE_H
#include "object.h"

class BaseModel: public BaseObject
{
public: 
	BaseModel(int width, int height, string const& path, bool gamma = false);
	~BaseModel();

public:
	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	Model* pModel = nullptr;
};
#endif //MODLE_BASE_H

