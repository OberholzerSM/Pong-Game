#version 460 core
out vec4 Frag_Colour;
uniform vec3 input_colour = vec3(0.9f,0.9f,0.9f);
void main()
{
	Frag_Colour = vec4(input_colour,1.0f);
}