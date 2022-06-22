#ifndef LIGHT_H
#define LIGHT_H
#include "object.h"

class Light : public BaseObject
{
public:
	Light(int width, int height) :
		BaseObject(width, height) {}
	~Light() {
		colors.clear();
	}

	virtual void init(Shader* pShader, Camera* pCamera);
	virtual void render();

	void setPrimaryColor(int index);
	void setColor(glm::vec3 color);	
	void adjustStrength(float value = 1.f);

protected:
	// color
	std::vector<glm::vec3> colors;
	glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
	float strength = 1.0;

};
#endif //LIGHT_H

