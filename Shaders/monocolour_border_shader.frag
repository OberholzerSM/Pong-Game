#version 460 core
in vec2 x_shape;
in vec2 x_border;
out vec4 Frag_Colour;
uniform vec3 input_colour = vec3(0.9f,0.9f,0.9f), border_colour = vec3(0.0f,0.0f,0.0f);
uniform vec2 border_thickness = vec2(0.0f,0.0f);

void main()
{
	if( abs(x_shape.x) >= x_border.x - border_thickness.x || abs(x_shape.y) >= x_border.y - border_thickness.y )
		Frag_Colour = vec4(border_colour,1.0f);
	else
		Frag_Colour = vec4(input_colour,1.0f);
}