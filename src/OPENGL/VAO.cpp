
#include <iostream>

#include "OPENGL/VAO.h"

VAO::VAO(){
    glGenVertexArrays(1, &this->id);
    glBindVertexArray(this->id);
}

VAO::VAO(GLuint id){
    this->id = id;
}

VAO::~VAO(){
    for(VBO*& vbo : this->vbos) delete vbo;
    glDeleteVertexArrays(1, &this->id);
}

void VAO::addVBO(VBO *vbo){
    this->bind();
    this->vbos.push_back(vbo);
    vbo->bind();
    glEnableVertexAttribArray(this->vbos.size() - 1);
    glVertexAttribPointer(this->vbos.size() - 1, vbo->getVertexSize(), vbo->getElementType(), false, 0, 0);
    glDisableVertexAttribArray(this->vbos.size() - 1);
}

GLuint VAO::getID(){
    return this->id;
}

size_t VAO::getVBOCount(){
    return this->vbos.size();
}

void VAO::bind(){
    glBindVertexArray(this->id);
}

void VAO::enableAttribArrays(){
    for(size_t i = 0; i != this->getVBOCount(); i++){
        glEnableVertexAttribArray(i);
    }
}

void VAO::disableAttribArrays(){
    for(size_t i = 0; i != this->getVBOCount(); i++){
        glDisableVertexAttribArray(i);
    }
}

void VAO::unbind(){
    glBindVertexArray(0);
}
