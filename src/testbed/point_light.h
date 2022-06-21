#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H
#include "object.h"

class PointLight : public BaseObject
{
public:
	PointLight(int width, int height) :
		BaseObject(width, height) {}
	~PointLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();
	void setColor(glm::vec3 color);

private:
	unsigned int sphereVAO = 0;
	int indexCount = 0;
	float scale = 0.05f;
	glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);

	void initSphere();
};
#endif //POINT_LIGHT_H

