
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "OPENGL/Matrix4f.h"

Matrix4f::Matrix4f(){}

Matrix4f::Matrix4f(float scale){
    float *matrix_data = this->array;
    matrix_data[0] = scale;
    matrix_data[1] = 0.0f;
    matrix_data[2] = 0.0f;
    matrix_data[3] = 0.0f;
    matrix_data[4] = 0.0f;
    matrix_data[5] = scale;
    matrix_data[6] = 0.0f;
    matrix_data[7] = 0.0f;
    matrix_data[8] = 0.0f;
    matrix_data[9] = 0.0f;
    matrix_data[10] = scale;
    matrix_data[11] = 0.0f;
    matrix_data[12] = 0.0f;
    matrix_data[13] = 0.0f;
    matrix_data[14] = 0.0f;
    matrix_data[15] = scale;
}

void Matrix4f::identity(){
    float *matrix_data = this->array;
    matrix_data[0] = 1.0f;
    matrix_data[1] = 0.0f;
    matrix_data[2] = 0.0f;
    matrix_data[3] = 0.0f;
    matrix_data[4] = 0.0f;
    matrix_data[5] = 1.0f;
    matrix_data[6] = 0.0f;
    matrix_data[7] = 0.0f;
    matrix_data[8] = 0.0f;
    matrix_data[9] = 0.0f;
    matrix_data[10] = 1.0f;
    matrix_data[11] = 0.0f;
    matrix_data[12] = 0.0f;
    matrix_data[13] = 0.0f;
    matrix_data[14] = 0.0f;
    matrix_data[15] = 1.0f;
}

void Matrix4f::transpose(){
    float *matrix_data = this->array;
    float a01 = matrix_data[1];
    float a02 = matrix_data[2];
    float a03 = matrix_data[3];
    float a12 = matrix_data[6];
    float a13 = matrix_data[7];
    float a23 = matrix_data[11];

    matrix_data[1] = matrix_data[4];
    matrix_data[2] = matrix_data[8];
    matrix_data[3] = matrix_data[12];
    matrix_data[4] = a01;
    matrix_data[6] = matrix_data[9];
    matrix_data[7] = matrix_data[13];
    matrix_data[8] = a02;
    matrix_data[9] = a12;
    matrix_data[11] = matrix_data[14];
    matrix_data[12] = a03;
    matrix_data[13] = a13;
    matrix_data[14] = a23;
}

float Matrix4f::determinant(){
    float *matrix_data = this->array;
	float a00 = matrix_data[0];
	float a01 = matrix_data[1];
	float a02 = matrix_data[2];
	float a03 = matrix_data[3];
	float a10 = matrix_data[4];
	float a11 = matrix_data[5];
	float a12 = matrix_data[6];
	float a13 = matrix_data[7];
	float a20 = matrix_data[8];
	float a21 = matrix_data[9];
	float a22 = matrix_data[10];
	float a23 = matrix_data[11];
	float a30 = matrix_data[12];
	float a31 = matrix_data[13];
	float a32 = matrix_data[14];
	float a33 = matrix_data[15];

	return (a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
	a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
	a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
	a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
	a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
	a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33);
}

void Matrix4f::inverse(){
    float *matrix_data = this->array;

	float a00 = matrix_data[0];
	float a01 = matrix_data[1];
	float a02 = matrix_data[2];
	float a03 = matrix_data[3];
	float a10 = matrix_data[4];
	float a11 = matrix_data[5];
	float a12 = matrix_data[6];
	float a13 = matrix_data[7];
	float a20 = matrix_data[8];
	float a21 = matrix_data[9];
	float a22 = matrix_data[10];
	float a23 = matrix_data[11];
	float a30 = matrix_data[12];
	float a31 = matrix_data[13];
	float a32 = matrix_data[14];
	float a33 = matrix_data[15];

	float b00 = a00 * a11 - a01 * a10;
	float b01 = a00 * a12 - a02 * a10;
	float b02 = a00 * a13 - a03 * a10;
	float b03 = a01 * a12 - a02 * a11;
	float b04 = a01 * a13 - a03 * a11;
	float b05 = a02 * a13 - a03 * a12;
	float b06 = a20 * a31 - a21 * a30;
	float b07 = a20 * a32 - a22 * a30;
	float b08 = a20 * a33 - a23 * a30;
	float b09 = a21 * a32 - a22 * a31;
	float b10 = a21 * a33 - a23 * a31;
	float b11 = a22 * a33 - a23 * a32;

	float d = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
	if(d == 0.0f) return;

	float inv_det = 1.0f / d;

	matrix_data[0] = (a11 * b11 - a12 * b10 + a13 * b09) * inv_det;
	matrix_data[1] = (-1.0f * a01 * b11 + a02 * b10 - a03 * b09) * inv_det;
	matrix_data[2] = (a31 * b05 - a32 * b04 + a33 * b03) * inv_det;
	matrix_data[3] = (-1.0f * a21 * b05 + a22 * b04 - a23 * b03) * inv_det;
	matrix_data[4] = (-1.0f * a10 * b11 + a12 * b08 - a13 * b07) * inv_det;
	matrix_data[5] = (a00 * b11 - a02 * b08 + a03 * b07) * inv_det;
	matrix_data[6] = (-1.0f * a30 * b05 + a32 * b02 - a33 * b01) * inv_det;
	matrix_data[7] = (a20 * b05 - a22 * b02 + a23 * b01) * inv_det;
	matrix_data[8] = (a10 * b10 - a11 * b08 + a13 * b06) * inv_det;
	matrix_data[9] = (-1.0f * a00 * b10 + a01 * b08 - a03 * b06) * inv_det;
	matrix_data[10] = (a30 * b04 - a31 * b02 + a33 * b00) * inv_det;
	matrix_data[11] = (-1.0f * a20 * b04 + a21 * b02 - a23 * b00) * inv_det;
	matrix_data[12] = (-1.0f * a10 * b09 + a11 * b07 - a12 * b06) * inv_det;
	matrix_data[13] = (a00 * b09 - a01 * b07 + a02 * b06) * inv_det;
	matrix_data[14] = (-1.0f * a30 * b03 + a31 * b01 - a32 * b00) * inv_det;
	matrix_data[15] = (a20 * b03 - a21 * b01 + a22 * b00) * inv_det;
}

void Matrix4f::toRotationMatrix(){
    float *matrix_data = this->array;
	matrix_data[12] = 0.0f;
	matrix_data[13] = 0.0f;
	matrix_data[14] = 0.0f;
	matrix_data[15] = 1.0f;
}

void Matrix4f::multiply(const Matrix4f& other){
    float *self_data = this->array;
	const float *other_data = other.array;

	float a00 = self_data[0];
	float a01 = self_data[1];
	float a02 = self_data[2];
	float a03 = self_data[3];
	float a10 = self_data[4];
	float a11 = self_data[5];
	float a12 = self_data[6];
	float a13 = self_data[7];
	float a20 = self_data[8];
	float a21 = self_data[9];
	float a22 = self_data[10];
	float a23 = self_data[11];
	float a30 = self_data[12];
	float a31 = self_data[13];
	float a32 = self_data[14];
	float a33 = self_data[15];

	float b00 = other_data[0];
	float b01 = other_data[1];
	float b02 = other_data[2];
	float b03 = other_data[3];
	float b10 = other_data[4];
	float b11 = other_data[5];
	float b12 = other_data[6];
	float b13 = other_data[7];
	float b20 = other_data[8];
	float b21 = other_data[9];
	float b22 = other_data[10];
	float b23 = other_data[11];
	float b30 = other_data[12];
	float b31 = other_data[13];
	float b32 = other_data[14];
	float b33 = other_data[15];

	self_data[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
	self_data[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
	self_data[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
	self_data[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;
	self_data[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
	self_data[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
	self_data[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
	self_data[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;
	self_data[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
	self_data[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
	self_data[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
	self_data[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;
	self_data[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
	self_data[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
	self_data[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
	self_data[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
}

void Matrix4f::translate(const Vector3f& xyz){
    float *matrix_data = this->array;
	float x = xyz.x;
	float y = xyz.y;
	float z = xyz.z;

	matrix_data[12] = matrix_data[0] * x + matrix_data[4] * y + matrix_data[8] * z + matrix_data[12];
	matrix_data[13] = matrix_data[1] * x + matrix_data[5] * y + matrix_data[9] * z + matrix_data[13];
	matrix_data[14] = matrix_data[2] * x + matrix_data[6] * y + matrix_data[10] * z + matrix_data[14];
	matrix_data[15] = matrix_data[3] * x + matrix_data[7] * y + matrix_data[11] * z + matrix_data[15];
}

void Matrix4f::translate(float x, float y, float z){
    float *matrix_data = this->array;
	
	matrix_data[12] = matrix_data[0] * x + matrix_data[4] * y + matrix_data[8] * z + matrix_data[12];
	matrix_data[13] = matrix_data[1] * x + matrix_data[5] * y + matrix_data[9] * z + matrix_data[13];
	matrix_data[14] = matrix_data[2] * x + matrix_data[6] * y + matrix_data[10] * z + matrix_data[14];
	matrix_data[15] = matrix_data[3] * x + matrix_data[7] * y + matrix_data[11] * z + matrix_data[15];
}

void Matrix4f::translateFromIdentity(const Vector3f& xyz){
    this->identity();
    this->translate(xyz);
}

void Matrix4f::translateFromIdentity(float x, float y, float z){
    this->identity();
    this->translate(x, y, z);
}

void Matrix4f::scale(const Vector3f& xyz){
    float *matrix_data = this->array;
	float x = xyz.x;
	float y = xyz.y;
	float z = xyz.z;

	matrix_data[0] = matrix_data[0] * x;
	matrix_data[1] = matrix_data[1] * x;
	matrix_data[2] = matrix_data[2] * x;
	matrix_data[3] = matrix_data[3] * x;
	matrix_data[4] = matrix_data[4] * y;
	matrix_data[5] = matrix_data[5] * y;
	matrix_data[6] = matrix_data[6] * y;
	matrix_data[7] = matrix_data[7] * y;
	matrix_data[8] = matrix_data[8] * z;
	matrix_data[9] = matrix_data[9] * z;
	matrix_data[10] = matrix_data[10] * z;
	matrix_data[11] = matrix_data[11] * z;
}

void Matrix4f::scale(float x, float y, float z){
    float *matrix_data = this->array;

	matrix_data[0] = matrix_data[0] * x;
	matrix_data[1] = matrix_data[1] * x;
	matrix_data[2] = matrix_data[2] * x;
	matrix_data[3] = matrix_data[3] * x;
	matrix_data[4] = matrix_data[4] * y;
	matrix_data[5] = matrix_data[5] * y;
	matrix_data[6] = matrix_data[6] * y;
	matrix_data[7] = matrix_data[7] * y;
	matrix_data[8] = matrix_data[8] * z;
	matrix_data[9] = matrix_data[9] * z;
	matrix_data[10] = matrix_data[10] * z;
	matrix_data[11] = matrix_data[11] * z;
}

void Matrix4f::scaleFromIdentity(const Vector3f& xyz){
    this->identity();
    this->scale(xyz);
}

void Matrix4f::scaleFromIdentity(float x, float y, float z){
    this->identity();
    this->scale(x, y, z);
}

void Matrix4f::rotate(float angle, const Vector3f& axis){
    float *matrix_data = this->array;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	float length = sqrtf(x * x + y * y + z * z);
    float s, c, t;

	if(length == 0.0f) return;

	if(length != 1.0f){
		length = 1.0f / length;
		x = x * length;
		y = y * length;
		z = z * length;
	}

	s = sinf(angle);
	c = cosf(angle);
	t = 1.0f - c;

	float a00 = matrix_data[0];
	float a01 = matrix_data[1];
	float a02 = matrix_data[2];
	float a03 = matrix_data[3];

	float a10 = matrix_data[4];
	float a11 = matrix_data[5];
	float a12 = matrix_data[6];
	float a13 = matrix_data[7];

	float a20 = matrix_data[8];
	float a21 = matrix_data[9];
	float a22 = matrix_data[10];
	float a23 = matrix_data[11];

	float b00 = x * x * t + c;
	float b01 = y * x * t + z * s;
	float b02 = z * x * t - y * s;
	float b10 = x * y * t - z * s;
	float b11 = y * y * t + c;
	float b12 = z * y * t + x * s;
	float b20 = x * z * t + y * s;
	float b21 = y * z * t - x * s;
	float b22 = z * z * t + c;

	matrix_data[0] = a00 * b00 + a10 * b01 + a20 * b02;
	matrix_data[1] = a01 * b00 + a11 * b01 + a21 * b02;
	matrix_data[2] = a02 * b00 + a12 * b01 + a22 * b02;
	matrix_data[3] = a03 * b00 + a13 * b01 + a23 * b02;

	matrix_data[4] = a00 * b10 + a10 * b11 + a20 * b12;
	matrix_data[5] = a01 * b10 + a11 * b11 + a21 * b12;
	matrix_data[6] = a02 * b10 + a12 * b11 + a22 * b12;
	matrix_data[7] = a03 * b10 + a13 * b11 + a23 * b12;

	matrix_data[8] = a00 * b20 + a10 * b21 + a20 * b22;
	matrix_data[9] = a01 * b20 + a11 * b21 + a21 * b22;
	matrix_data[10] = a02 * b20 + a12 * b21 + a22 * b22;
	matrix_data[11] = a03 * b20 + a13 * b21 + a23 * b22;
}

void Matrix4f::rotateFromIdentity(float angle, const Vector3f& axis){
    this->identity();
    this->rotate(angle, axis);
}

void Matrix4f::frustum(float left, float right, float bottom, float top, float near, float far){
    float *matrix_data = this->array;
	float rl = (right - left);
	float tb = (top - bottom);
	float fn = (far - near);

	matrix_data[0] = (near * 2.0f) / rl;
	matrix_data[1] = 0.0f;
	matrix_data[2] = 0.0f;
	matrix_data[3] = 0.0f;
	matrix_data[4] = 0.0f;
	matrix_data[5] = (near * 2.0f) / tb;
	matrix_data[6] = 0.0f;
	matrix_data[7] = 0.0f;
	matrix_data[8] = (right + left) / rl;
	matrix_data[9] = (top + bottom) / tb;
	matrix_data[10] = -1.0f * (far + near) / fn;
	matrix_data[11] = -1.0f;
	matrix_data[12] = 0.0f;
	matrix_data[13] = 0.0f;
	matrix_data[14] = -1.0f * (far * near * 2.0f) / fn;
	matrix_data[15] = 0.0f;
}

void Matrix4f::perspective(float fovy, float aspect, float near, float far){
    float top = near * tanf(fovy * 3.14159265358979323846f / 360.0f);
	float right = top * aspect;
	this->frustum(-1.0f * right, right, -1.0f * top, top, near, far);
}

void Matrix4f::ortho(float left, float right, float bottom, float top, float near, float far){
    float *matrix_data = this->array;
	float rl = (right - left);
	float tb = (top - bottom);
	float fn = (far - near);

	matrix_data[0] = 2.0f / rl;
	matrix_data[1] = 0.0f;
	matrix_data[2] = 0.0f;
	matrix_data[3] = 0.0f;
	matrix_data[4] = 0.0f;
	matrix_data[5] = 2.0f / tb;
	matrix_data[6] = 0.0f;
	matrix_data[7] = 0.0f;
	matrix_data[8] = 0.0f;
	matrix_data[9] = 0.0f;
	matrix_data[10] = -2.0f / fn;
	matrix_data[11] = 0.0f;
	matrix_data[12] = -1.0f * (left + right) / rl;
	matrix_data[13] = -1.0f * (top + bottom) / tb;
	matrix_data[14] = -1.0f * (far + near) / fn;
	matrix_data[15] = 1.0f;
}

void Matrix4f::lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up){
    float *matrix_data = this->array;

	float length;
	float eyex = eye.x;
	float eyey = eye.y;
	float eyez = eye.z;
	float upx = up.x;
	float upy = up.y;
	float upz = up.z;
	float centerx = center.x;
	float centery = center.y;
	float centerz = center.z;

	if(eyex == centerx && eyey == centery && eyez == centerz){
		this->identity();
	}

	float z0 = eyex - centerx;
	float z1 = eyey - centery;
	float z2 = eyez - centerz;

	length = 1.0f / sqrtf(z0 * z0 + z1 * z1 + z2 * z2);
	z0 = z0 * length;
	z1 = z1 * length;
	z2 = z2 * length;

	float x0 = upy * z2 - upz * z1;
	float x1 = upz * z0 - upx * z2;
	float x2 = upx * z1 - upy * z0;
	length = sqrtf(x0 * x0 + x1 * x1 + x2 * x2);

	if(length == 0.0f){
		x0 = 0.0f;
		x1 = 0.0f;
		x2 = 0.0f;
	} else {
		length = 1.0f / length;
		x0 = x0 * length;
		x1 = x1 * length;
		x2 = x2 * length;
	}

	float y0 = z1 * x2 - z2 * x1;
	float y1 = z2 * x0 - z0 * x2;
	float y2 = z0 * x1 - z1 * x0;
	length = sqrtf(y0 * y0 + y1 * y1 + y2 * y2);

	if(length == 0.0f){
		y0 = 0.0f;
		y1 = 0.0f;
		y2 = 0.0f;
	} else {
		length = 1.0f / length;
		y0 = y0 * length;
		y1 = y1 * length;
		y2 = y2 * length;
	}

	matrix_data[0] = x0;
	matrix_data[1] = y0;
	matrix_data[2] = z0;
	matrix_data[3] = 0.0f;
	matrix_data[4] = x1;
	matrix_data[5] = y1;
	matrix_data[6] = z1;
	matrix_data[7] = 0.0f;
	matrix_data[8] = x2;
	matrix_data[9] = y2;
	matrix_data[10] = z2;
	matrix_data[11] = 0.0f;
	matrix_data[12] = -1.0f * (x0 * eyex + x1 * eyey + x2 * eyez);
	matrix_data[13] = -1.0f * (y0 * eyex + y1 * eyey + y2 * eyez);
	matrix_data[14] = -1.0f * (z0 * eyex + z1 * eyey + z2 * eyez);
	matrix_data[15] = 1.0f;
}

void Matrix4f::print(){
    float *f = this->array;
	printf("Matrix4f {\n    %ff, %ff, %ff, %ff,\n    %ff, %ff, %ff, %ff,\n    %ff, %ff, %ff, %ff,\n    %ff, %ff, %ff, %ff}\n",
		f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9], f[10], f[11], f[12], f[13], f[14], f[15]);
}
