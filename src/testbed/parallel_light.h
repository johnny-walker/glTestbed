#ifndef PARALLEL_LIGHT_H
#define PARALLEL_LIGHT_H
#include "light.h"

class ParallelLight : public Light
{
private:
	glm::vec3 direction = glm::vec3(1.f, 1.f, 1.f);

public:
	ParallelLight(int width, int height) :
		Light(width, height) {}
	~ParallelLight() {}

	void init(Shader* pShader, Camera* pCamera);
	void render();
	void processInput(GLFWwindow* glWindow, float delta = 0);

	void setDirection(glm::vec3 dir);
	void updateDirection(float deltaX, float deltaY, float deltaZ);

};
#endif //PARALLEL_LIGHT_H

