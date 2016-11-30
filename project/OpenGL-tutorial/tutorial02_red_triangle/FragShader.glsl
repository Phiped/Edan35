#version 430
in vec2 tex_coords;

layout(RGBA32F) uniform image2D img_input;

out vec3 color;

void main(){
	tex_coords = tex_coords+vec2(1,1))/2.0;
	color = imageLoad(img_input,tex_coords.xy);
}
