#version 460 core
in vec2 texture_coordinates, data_form_out;
out vec4 Frag_Colour;
vec4 texture_colour;
uniform vec3 colour1 = vec3(0.5,0.78,0.87), colour2 = vec3(0.94,0.94,0.75);
uniform sampler2D texture_input;

void main()
{
	texture_colour = texture(texture_input,texture_coordinates);
	if(texture_colour.a < 0.1)
		Frag_Colour = mix( vec4(colour2,1.0), vec4(colour1,1.0) , data_form_out.y + 0.5 );
	else
		Frag_Colour = texture_colour;
}