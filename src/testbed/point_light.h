#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "light.h"

class PointLight : public Light
{
private:
	// drawing 
	unsigned int sphereVAO = 0;
	int indexCount = 0;
	float drawScale = 0.15f;

public:
	PointLight(int id, int width, int height) :
		Light(id, width, height) {}
	~PointLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	void initSphere();

};
#endif //POINT_LIGHT_H

