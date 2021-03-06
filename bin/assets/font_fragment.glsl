#version 400 core

in vec2 pass_uvs;
in vec3 pass_target_color;
out vec4 out_color;

uniform sampler2D texture_sampler;
uniform float y_upper_clip = 0.0; // NOTE: (0.0 is bottom and window_height is top)
uniform float x_right_clip = 0.0; // NOTE: (0.0 is left and window_height is right)

uniform float width; // = 0.45
uniform float edge;  // = 0.3

const float border_width = 0.0;
const float border_edge = 0.1;

const vec3 border_color = vec3(1.0, 0.0, 0.0);

void main(void){
    if((y_upper_clip != 0.0 && gl_FragCoord.y / gl_FragCoord.w > y_upper_clip)
    || (x_right_clip != 0.0 && gl_FragCoord.x / gl_FragCoord.w > x_right_clip)){
        discard;
    }
    
    float distance = 1.0 - texture(texture_sampler, pass_uvs).a;

    float alpha = 1.0 - smoothstep(width, width + edge, distance);
    float border_alpha = 1.0 - smoothstep(border_width, border_width + border_edge, distance);

    float total_alpha = alpha + (1.0 - alpha) * border_alpha;
    if(total_alpha == 0) discard;

    vec3 color = pass_target_color;
    vec3 total_color = mix(border_color, color, alpha / total_alpha);

    out_color = vec4(total_color, total_alpha);
}
