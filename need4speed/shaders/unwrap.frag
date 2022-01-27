#version 450 core

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texCoord;

out vec4 fs_color;

uniform sampler2D my_texture;

vec3 light_pos = vec3(-3, 3, 10);

void main()
{
	vec4 tex_color = texture(my_texture, vs_texCoord);

	float ambiant = 0.2f;

	vec3 light_dir = normalize(light_pos - vs_position);
	float diffuse = max(dot(vs_normal, light_dir), 0.0f);

	fs_color = vec4((ambiant + diffuse) * tex_color.rgb, 1.0);
}
