#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 texCoord;

out vec3 vs_position;
out vec3 vs_normal;
out vec4 vs_texCoord;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * vec4(position, 1.0);

	vs_position = position;
	vs_normal = normal;
	vs_texCoord = texCoord;
}
