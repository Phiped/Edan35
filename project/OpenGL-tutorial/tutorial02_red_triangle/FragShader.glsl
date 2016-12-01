#version 430
in vec2 tex_coords;

uniform sampler2D srcTex;

out vec4 color;

void main(){
	vec2 res = tex_coords+vec2(1,1)/2.0;
	color = texture(srcTex, res);
	//color = vec4(c, c, c, 1.0);
}