#version 460 core
in vec2 texture_coordinates;
out vec4 Frag_Colour;
vec4 texture_colour;
uniform vec3 input_colour = vec3(0.0f,0.0f,0.0f);
uniform sampler2D texture_input;

void main()
{
	texture_colour = texture(texture_input,texture_coordinates);
	if(texture_colour.a < 0.1f)
		discard;
	Frag_Colour = mix( texture_colour, vec4(input_colour,1.0f), 1.0f );
}