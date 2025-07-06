#version 460 core
in vec2 texture_coordinates;
out vec4 Frag_Colour;
vec4 texture_colour;
uniform sampler2D texture_input;

void main()
{
	texture_colour = texture(texture_input,texture_coordinates);
	if(texture_colour.a < 0.1f)
		discard;
	Frag_Colour = texture_colour;
}