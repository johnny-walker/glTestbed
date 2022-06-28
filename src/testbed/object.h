#ifndef BASEOBJ_H
#define BASEOBJ_H
#include "opengl.h"

class BaseObject
{
protected:
	Shader* pCurShader = nullptr;
	Camera* pCurCamera = nullptr;

	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	int scrWidth  = 0;
	int scrHeight = 0;
	
	float scale = 1.f;
	glm::vec3 angle = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 pos = glm::vec3(0.0f, 0.0f ,0.0f);
	bool dirty = true;	// flag if angle or pos changes

public:
	BaseObject(int width, int height) 
	{
		scrWidth  = width;
		scrHeight = height;
	}
	~BaseObject() {}

public:
	virtual void init(Shader* pShader, Camera* pCamera);
	virtual void render();
	virtual void setShader(Shader* pShader);

public:
	void setScale(float value);	
	void setAngle(float rotate, int axis = 1);   // axis {0:x, 1:y, 2:z}
	void updateAngle(float delta, int axis = 1); // axis {0:x, 1:y, 2:z}						
	void setPos(float posX, float posY, float posZ);
	void updatePos(float deltaX, float deltaY, float deltaZ);

	float getScale();
	glm::vec3 getAngle();
	glm::vec3 getPos();

	void processInput(GLFWwindow* glWindow, float delta);

protected:
	unsigned int loadTexture(char const* path);
};
#endif //BASEOBJ_H
