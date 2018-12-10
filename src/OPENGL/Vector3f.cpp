
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "OPENGL/Vector3f.h"

Vector3f::Vector3f(){
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
}

Vector3f::Vector3f(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector3f::set(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
}

void Vector3f::add(const Vector3f& other){
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
}

void Vector3f::subtract(const Vector3f& other){
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
}

void Vector3f::multiply(const Vector3f& other){
    this->x *= other.x;
    this->y *= other.y;
    this->z *= other.z;
}

void Vector3f::negate(){
    this->x = 0.0f - this->x;
    this->y = 0.0f - this->y;
    this->z = 0.0f - this->z;
}

void Vector3f::scale(float amount){
    this->x *= amount;
    this->y *= amount;
    this->z *= amount;
}

void Vector3f::normalize(){
    float length = sqrtf(x * x + y * y + z * z);

    if(length == 0.0f){
        this->set(0.0f, 0.0f, 0.0f);
        return;
    } else if(length == 1.0f){
        return;
    }

    length = 1.0f / length;
    this->set(x * length, y * length, z * length);
}

void Vector3f::cross(const Vector3f& other){
    float x_a = this->x;
    float y_a = this->y;
    float z_a = this->z;
    float x_b = other.x;
    float y_b = other.y;
    float z_b = other.z;

    this->x = y_a * z_b - z_a * y_b;
	this->y = z_a * x_b - x_a * z_b;
	this->z = x_a * y_b - y_a * x_b;
}

float Vector3f::length(){
    return sqrtf(x * x + y * y + z * z);
}

float Vector3f::dot(const Vector3f& other){
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

void Vector3f::direction(const Vector3f& other){
    float x_diff = this->x - other.x;
	float y_diff = this->y - other.y;
	float z_diff = this->z - other.z;
	float length = sqrtf(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);

	if(length == 0.0f){
        this->set(0.0f, 0.0f, 0.0f);
        return;
    }

	length = 1.0f / length;
	this->set(x_diff * length, y_diff * length, z_diff * length);
}

void Vector3f::lerp(const Vector3f& other){
    this->lerp(other, 0.5f);
}

void Vector3f::lerp(const Vector3f& other, float amount){
    this->x += amount * (other.x - this->x);
	this->y += amount * (other.y - this->y);
	this->z += amount * (other.z - this->z);
}

float Vector3f::distance(const Vector3f& other){
    float x_diff = this->x - other.x;
	float y_diff = this->y - other.y;
	float z_diff = this->z - other.z;
    return sqrtf(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);
}

void Vector3f::inverse(){
    this->x = 0.0f - this->x;
    this->y = 0.0f - this->y;
    this->z = 0.0f - this->z;
}

void Vector3f::print(){
    printf("Vector3f {%f, %f, %f}\n", this->x, this->y, this->z);
}
