
#ifndef VECTOR3F_H_INCLUDED
#define VECTOR3F_H_INCLUDED

class Vector3f {
public:
    float x;
    float y;
    float z;

    Vector3f();
    Vector3f(float x, float y, float z);
    void set(float x, float y, float z);
    void add(const Vector3f& other);
    void subtract(const Vector3f& other);
    void multiply(const Vector3f& other);
    void negate();
    void scale(float amount);
    void normalize();
    void cross(const Vector3f& other);
    float length();
    float dot(const Vector3f& other);
    void direction(const Vector3f& other);
    void lerp(const Vector3f& other);
    void lerp(const Vector3f& other, float amount);
    float distance(const Vector3f& other);
    void inverse();
    void print();
};

#endif // VECTOR3F_H_INCLUDED