
#ifndef SOLIDMODEL_H
#define SOLIDMODEL_H

#include <vector>
#include "VAO.h"
#include "EBO.h"

class SolidModel {
public:
    VAO *vao;
    EBO *ebo;
    
    SolidModel(std::vector<float>& vertices, std::vector<unsigned int>& indices);
    ~SolidModel();

    void draw();
};

SolidModel* createSolidModel(float w, float h);

#endif // SOLIDMODEL_H