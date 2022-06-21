#include "object.h"

void BaseObject::setAngle(float rotate) {
    angle = rotate;
    dirty = true;
}

void BaseObject::setPos(float posX, float posY, float posZ) {
    pos.x += posX;
    pos.y += posY;
    pos.z += posZ;
    dirty = true;
}

void BaseObject::updateAngle(float delta) {
    angle += delta;
    dirty = true;
}

void BaseObject::updatePos(float deltaX, float deltaY, float deltaZ) {
    pos.x += deltaX;
    pos.y += deltaY;
    pos.z += deltaZ;
    dirty = true;
}

unsigned int BaseObject::loadTexture(char const* path)
{
    unsigned int textureID=0;
    glGenTextures(1, &textureID);

    int width=0, height=0, nrComponents=3;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) {
            format = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
        } else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}