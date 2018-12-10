
#include <iostream>

#include "OPENGL/EBO.h"

EBO::EBO(const std::vector<unsigned>& unsignedData){
    this->size = unsignedData.size();

    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->size * sizeof(unsigned), unsignedData.data(), GL_STATIC_DRAW);
}

EBO::~EBO(){
    glDeleteBuffers(1, &this->id);
}

size_t EBO::getSize(){
    return this->size;
}

void EBO::bind(){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void EBO::unbind(){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
