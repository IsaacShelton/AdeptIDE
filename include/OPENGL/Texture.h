
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "OPENGL/Image.h"

enum TextureLoadOptions {
    ALPHA = 0x01,
    BLEND = 0x02,
    ALPHA_BLEND = 0x03
};

class Texture {
private:
    GLuint id;
    int width, height;
public:
    Texture(Image* image);
    Texture(const std::string& filename, TextureLoadOptions options);
    ~Texture();

    GLuint getID();
};

#endif // TEXTURE_H_INCLUDED