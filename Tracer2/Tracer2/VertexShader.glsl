#version 430

//Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;

out vec2 tex_coords;

void main(){

	tex_coords = vertexPosition_modelspace.xy;
    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;

}