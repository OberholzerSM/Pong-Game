#version 460 core
in vec2 data_form_out;
out vec4 Frag_Colour;
uniform vec3 colour1 = vec3(0.0,0.0,1.0), colour2 = vec3(0.5,0.5,1.0);

void main()
{
	Frag_Colour = mix( vec4(colour2,1.0), vec4(colour1,1.0) , data_form_out.y + 0.5 );
}