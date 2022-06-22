#ifndef LIGHT_H
#define LIGHT_H
#include "object.h"

class Light : public BaseObject
{
protected:
	// color
	std::vector<glm::vec3> colors;
	glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
	float strength = 1.0; 

public:
	Light(int width, int height) :
		BaseObject(width, height) {}
	~Light() {
		colors.clear();
	}

	virtual void init(Shader* pShader, Camera* pCamera);
	virtual void render();

	void processLight(GLFWwindow* glWindow);
	void setColor(glm::vec3 color);
	void setStrength(float value = 1.f);

private:
	int adjustLight = 0;	// flag to indicate{0:reset, 1:+, 2:-}

	void setPrimaryColor(int index);
	void adjustStrength(float delta = 1.f);
};
#endif //LIGHT_H

