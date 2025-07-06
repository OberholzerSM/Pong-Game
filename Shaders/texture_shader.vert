#version 460 core
layout(location=0) in vec2 data_form;
layout(location=1) in vec2 data_texture;
vec2 shape, shape_texture;
uniform vec2 x = vec2(0.0f, 0.0f);
uniform vec2 x_texture = vec2(0.0f, 0.0f);
uniform vec2 scaling = vec2(1.0f,1.0f);
uniform vec2 scaling_texture = vec2(1.0f,1.0f);
uniform bool flip = false;
mat2 mirror_matrix = mat2(-1.0,0.0,0.0,1.0);
out vec2 texture_coordinates;

void main()
{
	shape = data_form * scaling;
	shape_texture = data_texture * scaling_texture;
	if(flip)
	{
		shape = mirror_matrix*shape;
	}
	gl_Position = vec4(shape + x, 0.0f, 1.0f);
	texture_coordinates = shape_texture + x_texture;
}