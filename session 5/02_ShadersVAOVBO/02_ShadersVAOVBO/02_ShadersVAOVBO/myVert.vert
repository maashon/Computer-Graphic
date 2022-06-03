#version 130

// Variables coming from the VBO
in vec3 vs_in_pos;
in vec3 vs_in_col;
in vec2 vs_in_tex0;

// Values to be passed on the pipeline
out vec3 vs_out_pos;
out vec3 vs_out_col;
out vec2 vs_out_tex0;

void main()
{
	gl_Position = vec4( vs_in_pos, 1 );
	vs_out_pos = vs_in_pos;
	vs_out_col = vs_in_col;
	vs_out_tex0 = vs_in_tex0;
}