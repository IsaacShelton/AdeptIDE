
#include <iostream>

#include "OPENGL/VBO.h"

VBO::VBO(VBOContentType contentType, const std::vector<float>& floatData){
    this->contentType = contentType;
    this->size = floatData.size();
    
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
    glBufferData(GL_ARRAY_BUFFER, this->size * sizeof(float), floatData.data(), GL_STATIC_DRAW);
    VBO::unbind();
}

VBO::~VBO(){
    glDeleteBuffers(1, &this->id);
}

GLenum VBO::getElementType(){
    switch(this->contentType){
    case VERTICES:
    case TEXTURE_COORDS:
    case NORMALS:
        return GL_FLOAT;
    default:
        return GL_FLOAT;
    }
}

size_t VBO::getVertexSize(){
    switch(this->contentType){
    case VERTICES:
        return 3;
    case TEXTURE_COORDS:
        return 2;
    case NORMALS:
        return 3;
    default:
        std::cerr << "VBO::getVertexSize got unknown VBOContentType" << std::endl;
        return 3;
    }
}

size_t VBO::getSize(){
    return this->size;
}

void VBO::bind(){
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void VBO::unbind(){
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
