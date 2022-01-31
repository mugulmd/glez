#version 450 core

in vec3 vs_position;
in vec3 vs_normal;
in vec4 vs_texCoord;

out vec4 fs_color;

uniform sampler2D my_texture;

void main()
{
	vec2 tex_coord = vec2(vs_texCoord.x + vs_texCoord.z, 1.0 - (vs_texCoord.y + vs_texCoord.w));

	float level = textureQueryLod(my_texture, tex_coord).x;
	float scale = 1.0 / pow(2, level);

	ivec2 dim_base = textureSize(my_texture, 0);
	ivec2 dim_level = textureSize(my_texture, int(level));
	float dim_ratio = float(dim_base.x) / float(dim_level.x);

	vec2 tex_coord_scaled = vec2((scale*vs_texCoord.x+vs_texCoord.z)*dim_ratio, 1.0-(scale*vs_texCoord.y+vs_texCoord.w)*dim_ratio);

	vec4 tex_color = textureLod(my_texture, tex_coord_scaled, level);

	fs_color = tex_color;
}
