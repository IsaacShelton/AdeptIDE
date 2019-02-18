
#include <fstream>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "OPENGL/Texture.h"

Texture::Texture(Image *image){
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)image->data);
}

Texture::Texture(const std::string& filename, TextureLoadOptions options){
    int comp;
    unsigned char* image;

    if(options & TextureLoadOptions::ALPHA){
        image = stbi_load(filename.c_str(), &this->width, &this->height, &comp, STBI_rgb_alpha);
    } else {
        image = stbi_load(filename.c_str(), &this->width, &this->height, &comp, STBI_rgb);
    }

    if(image == NULL){
        std::ifstream access;
        access.open(filename.c_str());

        if(!access.is_open()){
            access.close();
            std::cerr << "Texture::load - File does not exist" << std::endl;
        } else {
            access.close();
            std::cerr << "Texture::load - Invalid image format" << std::endl;
        }
    }

    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if(options & TextureLoadOptions::BLEND){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    if(options & TextureLoadOptions::ALPHA){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*) image);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*) image);
    }

    stbi_image_free(image);
}

Texture::~Texture(){
    glDeleteTextures(1, &this->id);
}

GLuint Texture::getID(){
    return this->id;
}
