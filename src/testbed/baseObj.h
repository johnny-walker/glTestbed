#ifndef BASEOBJ_H
#define BASEOBJ_H
#include "opengl.h"

class BaseObject
{
public:
	Shader* objShader = NULL;

public:
	virtual void init(Shader* pShader = NULL)=0;
	virtual void render()=0;
	unsigned int loadTexture(char const* path);
};
#endif //BASEOBJ_H
