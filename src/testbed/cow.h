#ifndef Cow_H
#define Cow_H
#include "object.h"

class Cow: public BaseObject
{
public: 
	Cow(int width, int height, string const& path, bool gamma = false) :
		BaseObject(width, height)
	{
		spot = new Model(path, gamma);
	}

	~Cow() 
	{
		delete spot;
	}

public:
	void init(Shader* pShader, Camera* pCamera);
	void render();

private:
	Model* spot = NULL;
};
#endif //Cow

