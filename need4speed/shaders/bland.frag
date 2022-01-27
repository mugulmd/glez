#version 450 core

in vec2 vs_texCoord;

out vec4 fs_color;

void main()
{
	fs_color = vec4(0.8, 0.5, 0.1, 1.0);
}
