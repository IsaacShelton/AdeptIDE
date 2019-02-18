
#include <stdio.h>
#include "OPENGL/Vector4f.h"

Vector4f::Vector4f(){
    this->set(0.0f, 0.0f, 0.0f, 0.0f);
}

Vector4f::Vector4f(float x, float y, float z, float w){
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

void Vector4f::set(float x, float y, float z, float w){
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

void Vector4f::print(){
    printf("Vector4f {%f, %f, %f, %f}\n", this->x, this->y, this->z, this->w);
}