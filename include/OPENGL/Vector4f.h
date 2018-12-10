
#ifndef VECTOR4F_H_INCLUDED
#define VECTOR4F_H_INCLUDED

class Vector4f {
public:
    float x;
    float y;
    float z;
    float w;

    Vector4f();
    Vector4f(float x, float y, float z, float w);
    void set(float x, float y, float z, float w);
    void print();
};

#endif // VECTOR4F_H_INCLUDED