
#ifndef VBO_H_INCLUDED
#define VBO_H_INCLUDED

#include <vector>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum VBOContentType {
    VERTICES,
    TEXTURE_COORDS,
    NORMALS,
    OTHER
};

class VBO {
private:
    GLuint id;
    size_t size;
    VBOContentType contentType;
public:
    VBO(VBOContentType contentType, const std::vector<float>& floatData);
    ~VBO();
    GLenum getElementType();
    size_t getVertexSize();
    size_t getSize();
    void bind();

    static void unbind();
};

#endif // VBO_H_INCLUDED
