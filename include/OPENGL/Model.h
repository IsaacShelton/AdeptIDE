
#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include "VAO.h"
#include "EBO.h"
#include "Texture.h"

class Model {
    public:
    VAO *vao;
    EBO *ebo;
    Texture *texture;

    Model(VAO *vao, EBO *ebo, Texture *texture);
    ~Model();
};

class TextModel {
    public:
    unsigned int vao_id;
    unsigned int indices_vbo;
    unsigned int indices_count;
    unsigned int vertices_vbo;
    unsigned int uvs_vbo;
    unsigned int text_color_vbo;

    void free();
    void load(float*, size_t, unsigned int*, size_t, float*, size_t, float*, size_t);
    void draw();
};

void renderModel(Model *model);

#endif // MODEL_H_INCLUDED