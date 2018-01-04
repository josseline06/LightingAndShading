#ifndef _PROGRAM_H
#define _PROGRAM_H

#include "Definitions.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL\glew.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>
using namespace glm;

#include "Light.hpp"


extern mat4 view, projection;
extern mat4 mvp, mv; //Matriz ModelViewProjection, ModelView
extern mat3 normalMatrix; //NormalMatrix

extern Light lights[MAX_LIGHTS]; 

class Program{
public:
	GLuint id, vertex, fragment; 

	Program(){
		vertex = fragment = 0;
	}
	void Create(const char *vpath, const char *fpath){
		if(!CreateShader(vpath, GL_VERTEX_SHADER, vertex))
			return;
		if(!CreateShader(fpath, GL_FRAGMENT_SHADER, fragment))
			return;
		id = glCreateProgram();

		glAttachShader(id, vertex);
		glAttachShader(id, fragment);

		glLinkProgram(id);	
		PrintInfoLog(id);
	}
	char* ReadCode(const char *path){
		FILE *file;
		int size = 0;
		char *src = NULL;

		file = fopen(path, "r");

		if(file!=NULL){
			fseek(file,0, SEEK_END);
			size = ftell(file);
			rewind(file);
			src = (char*)malloc(size+1);
			size = (int)fread(src, 1, size, file);
			src[size]='\0';
			fclose(file);
		}
		return src;
	}
	void PrintInfoLog(GLuint id){
		int size = 0, reads;
		char *log;
		glGetObjectParameterivARB(id, GL_OBJECT_INFO_LOG_LENGTH_ARB, &size);
		if(size>0){
			log = (char*)malloc(size);
			glGetInfoLogARB(id, size, &reads, log);
			fprintf(stderr, "%s\n", log);
			free(log);
		}
	}
	GLint CreateShader(const char *path, GLenum type, GLuint &ids){
		const char* source = ReadCode(path);
		GLint compile = GL_FALSE;
		if(source==NULL){
			fprintf(stderr, "Error opening %s: ", path); perror("");
			return false;
		}
		ids = glCreateShader(type);
		glShaderSource(ids, 1, &source, NULL);
		glCompileShader(ids);
		PrintInfoLog(ids);
		glGetShaderiv(ids, GL_COMPILE_STATUS, &compile);
		return compile;
	}
	void Destroy(){
		glDetachShader(id, vertex);
		glDetachShader(id, fragment);
		glDeleteProgram(id);
	}
};

class Lighting : public Program {
private:
	GLint mvp_info, mv_info, n_info;
public:
	void GetUniforms(){
		GetUniformsMatrices();
		GetUniformsLights(ambient_info);
		for(int i=0; i<MAX_LIGHTS; ++i)
			GetUniformsLights(lights[i].info, i);
	}
	inline void GetUniformsMatrices(){
		mvp_info = glGetUniformLocation(id, "ModelViewProjection");
		mv_info = glGetUniformLocation(id, "ModelView");
		n_info = glGetUniformLocation(id, "NormalMatrix");
	}
	inline void GetUniformsLights(GLint & l_info){ 
		l_info = glGetUniformLocation(id, "ambient"); // Ambient light
	}
	inline void GetUniformsLights(LightIDS & l_info, int i){	
		char name[MAX_SHORT_BUFFER];

		sprintf(name, "Lights[%d].position", i); l_info.position = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].diffuse", i); l_info.diffuse = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].specular", i); l_info.specular = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].constantAtt", i); l_info.constantAtt = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].linearAtt", i); l_info.linearAtt = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].quadraticAtt", i); l_info.quadraticAtt = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].cutoff", i); l_info.cutoff = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].exponent", i); l_info.exponent = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].direction", i); l_info.direction = glGetUniformLocation(id, name);
		sprintf(name, "Lights[%d].active", i); l_info.active = glGetUniformLocation(id, name);
	}
	inline void SendUniformsMatricesToShader(){
		glUniformMatrix4fv(mvp_info, 1, GL_FALSE, &mvp[0][0]); //modelviewprojection
		glUniformMatrix4fv(mv_info, 1, GL_FALSE, &mv[0][0]); //modelview
		glUniformMatrix3fv(n_info, 1, GL_FALSE, &normalMatrix[0][0]); //normalmatrix
	}
	inline void SendUniformsLightsToShader(vec3 & l_ambient, GLint uid){
		glUniform3fv(uid, 1, &l_ambient[0]);
	}
	inline void SendUniformsLightsToShader(Light & l){
		vec4 position = view*l.position;
		glUniform4fv(l.info.position, 1, &position[0]);
		glUniform3fv(l.info.diffuse, 1, &l.diffuse[0]);
		glUniform3fv(l.info.specular, 1, &l.specular[0]);
		glUniform1fv(l.info.constantAtt, 1, &l.constantAtt);
		glUniform1fv(l.info.linearAtt, 1, &l.linearAtt);
		glUniform1fv(l.info.quadraticAtt, 1, &l.quadraticAtt);
		glUniform1fv(l.info.cutoff, 1, &l.cutoff);
		glUniform1fv(l.info.exponent, 1, &l.exponent);
		glUniform3fv(l.info.direction, 1, &l.direction[0]);
		glUniform1iv(l.info.active, 1, &l.active);
	}
};

class Standar : public Program {
private:
	GLint mvp_info, color_info;
public:
	void GetUniforms(){
		mvp_info = glGetUniformLocation(id, "ModelViewProjection");
		color_info = glGetUniformLocation(id, "color");
	}
	void SendUniformsToShader(vec4 color){
		glUniformMatrix4fv(mvp_info, 1, GL_FALSE, &mvp[0][0]); //modelviewprojection
		glUniform4fv(color_info, 1, &color[0]);
	}
};
#endif