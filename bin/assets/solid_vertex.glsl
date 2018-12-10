#version 400 core

in vec3 position;

uniform mat4 projection_matrix;
uniform mat4 transformation_matrix;

void main(void){
    gl_Position = projection_matrix * transformation_matrix * vec4(position.xyz, 1.0);
}
