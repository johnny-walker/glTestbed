#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "light.h"

class PointLight : public Light
{
public:
	PointLight(int width, int height) :
		Light(width, height) {}
	~PointLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	// drawing 
	unsigned int sphereVAO = 0;
	int indexCount = 0;
	float scale = 0.1f;

	void initSphere();
};
#endif //POINT_LIGHT_H

