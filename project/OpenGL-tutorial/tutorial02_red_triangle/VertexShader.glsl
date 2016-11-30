#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in sampler2Dtexture texture;

void main(){

    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;

}
