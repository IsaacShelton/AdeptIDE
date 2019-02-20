
#ifndef MATRIX4F_H_INCLUDED
#define MATRIX4F_H_INCLUDED

#include "Vector3f.h"

class Matrix4f {
public:
    float array[16];

    Matrix4f();
    Matrix4f(float scale);
    void identity();
    void transpose();
    float determinant();
    void inverse();
    void toRotationMatrix();
    void multiply(const Matrix4f& other);
    void translate(const Vector3f& xyz);
    void translate(float x, float y, float z);
    void translateFromIdentity(const Vector3f& xyz);
    void translateFromIdentity(float x, float y, float z);
    void scale(const Vector3f& xyz);
    void scale(float x, float y, float z);
    void scaleFromIdentity(const Vector3f& xyz);
    void scaleFromIdentity(float x, float y, float z);
    void rotate(float angle, const Vector3f& axis);
    void rotateFromIdentity(float angle, const Vector3f& axis);
    void frustum(float left, float right, float bottom, float top, float near, float far);
    void perspective(float fovy, float aspect, float near, float far);
    void ortho(float left, float right, float bottom, float top, float near, float far);
    void lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up);
    void print();
};

#endif // MATRIX4F_H_INCLUDED