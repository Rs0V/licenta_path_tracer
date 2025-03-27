#version 450 core
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

out vec2 texcoord;

void main() {
    texcoord = vec2(in_texcoord.x, 1.0 - in_texcoord.y);
    gl_Position = vec4(in_position, 0.0, 1.0);
}
