#ifndef _LIGHT_H
#define _LIGHT_H

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
using namespace glm;

#include "Icosphere.hpp"

GLuint light_vao;
Icosphere source;
mat4 light_model;

mat4 box_light_model;
GLuint light_box_vao;

void TemplateLight(){ 
	GLuint vbo, ibo; 

	glGenBuffers(1, &vbo); 
	glGenBuffers(1, &ibo); 
	glGenVertexArrays(1, &light_vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*source.vertices.size(), source.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*source.indices.size(), source.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(light_vao);
	glEnableVertexAttribArray(attrib_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	light_model = scale(mat4(1.f), vec3(0.08f)); // Transformation of light source's model
	box_light_model = translate(mat4(1.f), (source.maxi+source.mini)*0.5f)*scale(mat4(1.f), source.maxi-source.mini);
}

// Ambient light
GLint ambient_info;
vec3 ambient;

struct LightIDS{
	GLint position;
	GLint direction; 
	GLint diffuse;
	GLint specular;
	GLint constantAtt;
	GLint linearAtt;
	GLint quadraticAtt;
	GLint cutoff;
	GLint exponent;
	GLint active;
};

class Light{
public:
	vec4 position;
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
	float constantAtt;  
	float linearAtt;  
	float quadraticAtt;  
	float last_cutoff, cutoff; // focus openning (in dregrees)
	float exponent; //light intensity unfocused [1,128]
	int active;
	int type;
	int id;
	LightIDS info;

	Light(void){
		position = vec4(vec3(0.f), 0.f);
		diffuse = specular = vec3(0.f);
		direction = vec3(0.f, 1.f, -1.f);
		exponent = 0.f;
		constantAtt = 1.f;
		linearAtt = quadraticAtt = 0.f;
		last_cutoff = 1.f;
		cutoff = 180.f;
		active = false;
	}
	mat4 ModelMatrix(){ return translate(mat4(1.f), vec3(position))*light_model; }
	vec4 GetColor(){ 
		vec3 mixed = 0.5f*(diffuse+specular);
		return vec4(0.33f*ambient + 0.67f*mixed, 0.6f); 
	}
	void Draw(){
		glBindVertexArray(light_vao);
		glDrawElements(GL_TRIANGLES, source.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

#endif

