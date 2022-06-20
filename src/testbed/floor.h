#ifndef FLOOR_H
#define FLOOR_H

#include "baseObj.h"
class Floor: public BaseObject
{
	unsigned int planeVAO = 0;
	unsigned int woodTexture = 0;

public:
	Floor() {}
	void init(Shader* pShader = NULL);
	void render();
};

#endif //FLOOR_H

