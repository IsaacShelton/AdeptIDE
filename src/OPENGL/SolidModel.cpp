
#include <assert.h>
#include "OPENGL/SolidModel.h"

SolidModel::SolidModel(std::vector<float>& vertices, std::vector<unsigned int>& indices){
    this->vao = new VAO();
    this->vao->bind();

    VBO *verticesVBO = new VBO(VBOContentType::VERTICES, vertices);
    this->vao->addVBO(verticesVBO);

    this->ebo = new EBO(indices);
}

SolidModel::~SolidModel(){
    delete this->vao;
    delete this->ebo;
}

void SolidModel::draw(){
    assert(this->vao);
    assert(this->ebo);

    this->vao->bind();
    this->vao->enableAttribArrays();
    this->ebo->bind();

    glDrawElements(GL_TRIANGLES, this->ebo->getSize(), GL_UNSIGNED_INT, 0);

    EBO::unbind();
    this->vao->disableAttribArrays();
    VAO::unbind();
}

SolidModel* createSolidModel(float w, float h){
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back( 0.0f);
    vertices.push_back(h);
    vertices.push_back( 0.0f);
    vertices.push_back( 0.0f);
    vertices.push_back( 0.0f);
    vertices.push_back( 0.0f);
    vertices.push_back(w);
    vertices.push_back( 0.0f);
    vertices.push_back( 0.0f);
    vertices.push_back(w);
    vertices.push_back(h);
    vertices.push_back( 0.0f);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(3);
    indices.push_back(1);
    indices.push_back(2);

    return new SolidModel(vertices, indices);
}
