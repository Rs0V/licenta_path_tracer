#version 450 core
in vec2 texcoord;
out vec4 frag_color;

uniform sampler2D tex_output;

void main() {
    frag_color = texture(tex_output, texcoord);
}
