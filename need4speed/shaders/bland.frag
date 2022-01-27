#version 450 core

in vec2 vs_texCoord;

out vec4 fs_color;

uniform sampler2D my_texture;

void main()
{
	fs_color = texture(my_texture, vs_texCoord);
}
