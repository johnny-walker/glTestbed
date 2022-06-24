#ifndef CUBE_H
#define CUBE_H
#include "object.h"

class Cube : public BaseObject
{
public:
	Cube(int width, int height) :
		BaseObject(width, height) {}
	~Cube() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	unsigned int cubeVAO = 0;
	unsigned int woodTexture = 0;
	unsigned int specularTexture = 0;
};

#endif //CUBE_H

