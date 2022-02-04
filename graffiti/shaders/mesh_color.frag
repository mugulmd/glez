#version 450 core

in vec3 vs_position;
in vec3 vs_normal;
in vec4 vs_texCoord;

out vec4 fs_color;

uniform sampler2D my_texture;

void main()
{
	vec2 tex_coord = vec2(vs_texCoord.x + vs_texCoord.z, 1.0 - (vs_texCoord.y + vs_texCoord.w));

	vec4 tex_color = texture(my_texture, tex_coord);

	fs_color = tex_color;
}
