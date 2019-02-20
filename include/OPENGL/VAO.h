
#ifndef VAO_H_INCLUDED
#define VAO_H_INCLUDED

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "VBO.h"

class VAO {
private:
    GLuint id;
    std::vector<VBO*> vbos;
public:    
    VAO();
    VAO(GLuint id);
    ~VAO();
    void addVBO(VBO *vbo);
    GLuint getID();
    size_t getVBOCount();
    void bind();
    void enableAttribArrays();
    void disableAttribArrays();

    static void unbind();
};

#endif // VAO_H_INCLUDED