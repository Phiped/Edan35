#version 430
in vec2 tex_coords;

uniform sampler2D srcTex;

out vec4 color;

void main(){
	vec2 res = tex_coords * 0.5 + 0.5;
	color = texture(srcTex, res);
}