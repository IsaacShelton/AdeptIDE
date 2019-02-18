
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>

#include "Vector3f.h"
#include "Vector4f.h"
#include "Matrix4f.h"

class Shader {
    private:
    GLuint program;
    GLuint vertex;
    GLuint fragment;

    GLint getUniformLocation(const std::string& name);

    public:
    Shader(const std::string& vertexFile, const std::string& fragmentFile, const std::vector<std::string>& attributes);
    ~Shader();
    void bind();
    void giveFloat(const std::string& uniform, float value);
    void giveVector3f(const std::string& uniform, const Vector3f& value);
    void giveVector4f(const std::string& uniform, const Vector4f& value);
    void giveMatrix4f(const std::string& unuform, const Matrix4f& value);
};

#endif // SHADER_H_INCLUDED