#version 400

layout(location = 0) in vec3 vertex;
uniform mat4 ModelViewProjection;

void main(){
	gl_Position = ModelViewProjection*vec4(vertex,1.f);
}