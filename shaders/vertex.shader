#version 460

layout (location = 0) in vec3 pos;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(0.5 * pos.x, 0.5 * pos.y, pos.z, 1.0);
}
