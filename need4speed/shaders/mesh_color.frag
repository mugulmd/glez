#version 450 core

in vec3 vs_position;
in vec3 vs_normal;
in vec4 vs_texCoord;

out vec4 fs_color;

uniform sampler2D my_texture;

vec3 light_pos = vec3(-3, 3, 10);

void main()
{
	vec2 tex_coord = vec2(vs_texCoord.x + vs_texCoord.z, 1.0 - (vs_texCoord.y + vs_texCoord.w));
	float level = textureQueryLod(my_texture, tex_coord);
	float scale = 1.0 / pow(2, level);
	vec2 tex_coord_scaled = vec2(scale * vs_texCoord.x + vs_texCoord.z, 1.0 - (scale * vs_texCoord.y + vs_texCoord.w));
	vec4 tex_color = texture(my_texture, tex_coord_scaled);

	float ambiant = 1.0;

	vec3 light_dir = normalize(light_pos - vs_position);
	float diffuse = max(dot(vs_normal, light_dir), 0.0f);

	fs_color = vec4(0.5 * (ambiant + diffuse) * tex_color.rgb, 1.0);
}
