
#ifndef EBO_H_INCLUDED
#define EBO_H_INCLUDED

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

class EBO {
private:
    GLuint id;
    size_t size;
public:
    EBO(const std::vector<unsigned>& unsignedData);
    ~EBO();
    size_t getSize();
    void bind();

    static void unbind();
};

#endif // EBO_H_INCLUDED