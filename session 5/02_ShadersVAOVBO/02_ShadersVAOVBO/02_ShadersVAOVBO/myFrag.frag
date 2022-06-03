#version 130

in vec3 vs_out_pos;
in vec3 vs_out_col;
in vec2 vs_out_tex0;

out vec4 fs_out_col;

uniform sampler2D texImage;

void main()
{
	fs_out_col = vec4(vs_out_col, 1);
	fs_out_col = texture(texImage, vs_out_tex0);
}