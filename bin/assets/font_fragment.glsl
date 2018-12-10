#version 400 core

in vec2 pass_uvs;
in vec3 pass_target_color;
out vec4 out_color;

uniform sampler2D texture_sampler;

uniform float width; // = 0.45
uniform float edge;  // = 0.3

const float border_width = 0.0;
const float border_edge = 0.1;

const vec3 border_color = vec3(1.0f, 0.0f, 0.0f);

void main(void){
    float distance = 1.0 - texture(texture_sampler, pass_uvs).a;

    float alpha = 1.0 - smoothstep(width, width + edge, distance);
    float border_alpha = 1.0 - smoothstep(border_width, border_width + border_edge, distance);

    float total_alpha = alpha + (1.0 - alpha) * border_alpha;
    if(total_alpha == 0) discard;

    vec3 color = pass_target_color;
    vec3 total_color = mix(border_color, color, alpha / total_alpha);

    out_color = vec4(total_color, total_alpha);
}
