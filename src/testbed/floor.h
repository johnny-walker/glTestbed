#ifndef FLOOR_H
#define FLOOR_H
#include "object.h"

class Floor: public BaseObject
{
public:
	Floor(int width, int height) :
		BaseObject(width, height)
	{}

	~Floor() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();
	void updateAngle(float angle);
	void updatePos(float x, float y, float z);

private:
	unsigned int planeVAO = 0;
	unsigned int woodTexture = 0;
};

#endif //FLOOR_H

