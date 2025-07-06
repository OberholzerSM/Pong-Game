#version 460 core
layout(location=0) in vec2 x_start;
vec2 x_shape;
uniform vec2 x = vec2(0.0f, 0.0f);
uniform vec2 scaling = vec2(1.0f,1.0f);
void main()
{
	x_shape = x_start * scaling;
	gl_Position = vec4(x_shape + x, 0.0f, 1.0f);
}