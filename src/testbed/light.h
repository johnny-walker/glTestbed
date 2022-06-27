#ifndef LIGHT_H
#define LIGHT_H
#include "object.h"

class Light : public BaseObject
{
protected:
	int identifier = 0;
	glm::mat4 lightSpaceMtrx = glm::mat4(1.f);

	// color
	std::vector<glm::vec3> colors;
	glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
	float strength = 1.0; 

public:
	Light(int id, int width, int height);
	~Light();

	virtual void init(Shader* pShader, Camera* pCamera);
	virtual void render();

	glm::mat4 createLightSpaceMatrix(float nearPlane, float farPlane);
	glm::mat4 getLightSpaceMatrix();

	int getId(int index);
	void setColor(glm::vec3 color);
	void setStrength(float value = 1.f);
	void setPrimaryColor(int index);
	void adjustStrength(float delta = 1.f);
	void processLight(GLFWwindow* glWindow);

private:
	int adjustLight = 0;	// control flag {0:reset, 1:+, 2:-}
};
#endif //LIGHT_H

