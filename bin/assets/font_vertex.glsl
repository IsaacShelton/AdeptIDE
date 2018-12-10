#version 400 core

in vec2 position;
in vec2 uvs;
layout (location = 2) in vec3 target_color;

out vec2 pass_uvs;
out vec3 pass_target_color;

uniform mat4 projection_matrix;
uniform mat4 transformation_matrix;

void main(void){
    gl_Position = projection_matrix * transformation_matrix * vec4(position.xy, 0.0f, 1.0);
    pass_uvs = uvs;
    pass_target_color = target_color;
}
