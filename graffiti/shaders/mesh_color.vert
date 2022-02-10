#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 texCoord;

uniform mat4 view;
uniform mat4 projection;

vec3 light_pos = vec3(0, 0, -10);

out vec3 vs_position;
out vec3 vs_normal;
out vec4 vs_texCoord;
out vec3 vs_light_pos;

void main()
{
	gl_Position = projection * view * vec4(position, 1.0);

	vs_position = position;
	vs_normal = normal;
	vs_texCoord = texCoord;

	vs_light_pos = vec3(projection * view * vec4(light_pos, 1.0));
}
