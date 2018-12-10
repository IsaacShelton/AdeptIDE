
#include <assert.h>
#include "OPENGL/Model.h"

Model::Model(VAO *vao, EBO *ebo, Texture *texture){
    this->vao = vao;
    this->ebo = ebo;
    this->texture = texture;
}

Model::~Model(){
    delete vao;
    delete ebo;
}

void TextModel::free(){
    glDeleteBuffers(1, &indices_vbo);
    glDeleteBuffers(1, &vertices_vbo);
    glDeleteBuffers(1, &uvs_vbo);
    glDeleteBuffers(1, &text_color_vbo);
    glDeleteVertexArrays(1, &vao_id);
}

void TextModel::load(float* positions, size_t positions_length, unsigned int* indices, size_t indices_length,
                  float* texture_coords, size_t texture_coords_length, float* text_colors, size_t text_colors_length){
    glGenVertexArrays(1, &(this->vao_id));
    glBindVertexArray(this->vao_id);
    this->indices_count = indices_length;

    glGenBuffers(1, &vertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions_length * sizeof(float), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0 /*index in vao*/, 2 /*vertex size*/, GL_FLOAT /*type*/, false /*normalized*/, 0 /*stride*/, (void*) 0 /*offset*/);

    glGenBuffers(1, &uvs_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvs_vbo);
    glBufferData(GL_ARRAY_BUFFER, texture_coords_length * sizeof(float), texture_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1 /*index in vao*/, 2 /*vertex size*/, GL_FLOAT /*type*/, false /*normalized*/, 0 /*stride*/, (void*) 0 /*offset*/);

    glGenBuffers(1, &text_color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, text_color_vbo);
    glBufferData(GL_ARRAY_BUFFER, text_colors_length * sizeof(float), text_colors, GL_STATIC_DRAW);
    glVertexAttribPointer(2 /*index in vao*/, 3 /*vertex size*/, GL_FLOAT /*type*/, false /*normalized*/, 0 /*stride*/, (void*) 0 /*offset*/);

    glGenBuffers(1, &(this->indices_vbo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indices_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_length * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextModel::draw(){
    glBindVertexArray(this->vao_id);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indices_vbo);
    glDrawElements(GL_TRIANGLES, this->indices_count, GL_UNSIGNED_INT, (void*) 0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void renderModel(Model *model){
    assert(model->vao);
    assert(model->ebo);

    model->vao->bind();
    model->vao->enableAttribArrays();
    model->ebo->bind();

    if(model->texture){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->texture->getID());
    }

    glDrawElements(GL_TRIANGLES, model->ebo->getSize(), GL_UNSIGNED_INT, 0);

    EBO::unbind();
    model->vao->disableAttribArrays();
    VAO::unbind();
}
