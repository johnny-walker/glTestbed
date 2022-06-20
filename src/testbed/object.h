#ifndef BASEOBJ_H
#define BASEOBJ_H
#include "opengl.h"

class BaseObject
{
public:
	Shader* pCurShader = NULL;
	Camera* pCurCamera = NULL;

	int scr_width = 0;
	int scr_height = 0;

	float angle = 0;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f ,0.0f);

public:
	BaseObject(int width, int height) {
		scr_width = width;
		scr_height = height;
	}
	~BaseObject() {

	}

public:
	virtual void init(Shader* pShader, Camera* pCamera) = 0;
	virtual void render() = 0;
	virtual void updateAngle(float angle) = 0;
	virtual void updatePos(float x, float y, float z) = 0;

	unsigned int loadTexture(char const* path);
};
#endif //BASEOBJ_H
