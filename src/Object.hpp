#ifndef _OBJECT_H
#define _OBJECT_H

#include "Definitions.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <vector>
using namespace std; 

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\norm.hpp>
using namespace glm;

#include "Material.hpp"

GLuint model_box_vao;

struct Vertex{
	vec3 point;
	vec3 normal;
	Material material;

	Vertex(){}
	Vertex(vec3 &p, vec3 &n, Material &m){
		point = p;
		normal = n;
		material = m;
	}
	bool operator<(const Vertex other) const{
		return memcmp((void*)this, (void*)&other, sizeof(Vertex))>0;
	};
};

// Auxiliars buffers
map <string,unsigned int> materials_buffer;
map <Vertex,unsigned int> out_buffer;
vector <vec3> in_vertices, in_normal; 
vector <Material> in_materials;
vector <int> v_indices, n_indices, m_indices; 

class Object{
public:
	char name[MAX_SIZE_NAME];
	bool animation;
	float animation_speed;
	GLuint vao;
	vector <vec3> vertices;
	vector <vec3> normal;
	vector <GLuint> indices;
	vector <Material> materials;
	vec3 maxi, mini;
	mat4 box_model;
	quat init_r, rotation;
	vec3 init_s, scaling, init_t, translation;
	int shading;
	int lighting;

	Object(){
		shading = gouraud;
		lighting = _phong;
		animation = false;
		animation_speed = 0.2f;
		maxi = vec3(-inf);
		mini = vec3(inf);
		scaling = vec3(1.f);
	}
	void Init(const char *obj_path, const char *mtl_path){
		LoadMTL(mtl_path);
		LoadOBJ(obj_path);
		Indexer();
		Normalize();
		Upload();
		Start();
		box_model = translate(mat4(1.f), (maxi+mini)*0.5f)*scale(mat4(1.f), maxi-mini);
	
		// Cleaning all
		materials_buffer.clear();
		out_buffer.clear();
		in_vertices.clear();
		in_normal.clear();
		in_materials.clear();
		v_indices.clear();
		n_indices.clear();
		m_indices.clear(); 
	}
	void LoadMTL(const char *path){
		FILE* file;
		int bytes;
		char buffer[MAX_BUFFER], aux[MAX_SHORT_BUFFER];
		string current;
		int count = -1;

		file = fopen(path, "r");
		
		if(file==NULL) return;
		while(!feof(file)){
			fscanf(file, "%[^\n]%*[\n]", buffer);
			if(sscanf(buffer, "%s%*[\t\r ]%n", aux, &bytes)<1)
				continue;
			if(strcmp("newmtl", aux)==0){
				sscanf(buffer+bytes, "%s", aux);
				current = aux;
				materials_buffer[current] = ++count;
				in_materials.push_back(Material());
				continue;
			}
			if(strcmp("Ka", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &in_materials[count].ambient.r, &in_materials[count].ambient.g, &in_materials[count].ambient.b);
				continue;
			}
			if(strcmp("Kd", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &in_materials[count].diffuse.r, &in_materials[count].diffuse.g, &in_materials[count].diffuse.b);
				continue;
			}
			if(strcmp("Ks", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &in_materials[count].specular.r, &in_materials[count].specular.g, &in_materials[count].specular.b);
				continue;
			}
			if(strcmp("Ns", aux)==0){
				sscanf(buffer+bytes, "%f", &in_materials[count].shininess);
				continue;
			}
			if(strcmp("Ft", aux)==0){
				sscanf(buffer+bytes, "%f", &in_materials[count].reflectance);
			}
		}
		fclose(file);
	}
	void LoadOBJ(const char *path){
		FILE* file;
		char buffer[MAX_BUFFER], aux[MAX_SHORT_BUFFER];
		int bytes, bytes_reads;
		GLuint vindex, nindex;
		vec3 element;
		string current;
	
		file = fopen(path, "r");
		
		if(file==NULL) return;
		
		while(!feof(file)){
			fscanf(file, "%[^\n]%*[\n]", buffer);
			if(sscanf(buffer, "%s%*[\t\r ]%n", aux, &bytes)<1)
				continue;
			if(strcmp("v", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &element.x, &element.y, &element.z);
				in_vertices.push_back(element);
				mini = vec3(MIN(mini.x,element.x), MIN(mini.y,element.y), MIN(mini.z,element.z));
				maxi = vec3(MAX(maxi.x,element.x), MAX(maxi.y,element.y), MAX(maxi.z,element.z));
				continue;
			}
			if(strcmp("vn", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &element.x, &element.y, &element.z);
				in_normal.push_back(element);
			}
			if(strcmp("usemtl", aux)==0){
				sscanf(buffer+bytes, "%s", aux);
				current = aux;
				continue;
			}
			if(strcmp("f", aux)==0){
				for(int i=0;i<3;++i){
					sscanf(buffer+bytes, "%d//%d%n", &vindex, &nindex, &bytes_reads);
					vindex--; nindex--;
					v_indices.push_back(vindex);
					n_indices.push_back(nindex);
					m_indices.push_back(materials_buffer[current]);
					bytes += bytes_reads;
				}
				continue;
			}
		}
		fclose(file);
	}
	void Indexer(){
		int n = v_indices.size();
		GLuint index;
		Vertex pack;
		map <Vertex, unsigned int>::iterator it;

		for(int i=0; i<n; ++i){
			pack = Vertex(in_vertices[v_indices[i]], in_normal[n_indices[i]], in_materials[m_indices[i]]);
			it = out_buffer.find(pack);
			if(it == out_buffer.end()){
				vertices.push_back(in_vertices[v_indices[i]]);
				normal.push_back(in_normal[n_indices[i]]);
				materials.push_back(in_materials[m_indices[i]]);
				index = vertices.size()-1;
				out_buffer[pack] = index;
			}else
				index = it->second;
			indices.push_back(index);
		}
	}
	void Normalize(){
		int n = vertices.size();
		float delta, norm;
		vec3 center, deltav; 

		deltav = maxi - mini;
		norm = INFINITY_NORM(deltav.x, deltav.y, deltav.z);
		if(norm>0){
			delta = 1.f/norm;
			center = (maxi+mini)*0.5f;
			for(int i=0; i<n; ++i)
				vertices[i] = (vertices[i] - center)*delta;
			mini = (mini-center)*delta;
			maxi = (maxi-center)*delta;
		}
	}
	inline void Start() { 
		rotation = init_r;
		translation = init_t;
		scaling = init_s;
	}
	inline mat4 ModelMatrix() { return translate(mat4(1.f), translation)*toMat4(rotation)*scale(mat4(1.f), scaling); }
	void Upload(){
		GLuint ibo, vbo[3];
		
		glGenBuffers(3, vbo);
		glGenBuffers(1, &ibo);
		glGenVertexArrays(1, &vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // Vertices
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec3), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // Normal vectors
		glBufferData(GL_ARRAY_BUFFER, normal.size()*sizeof(vec3), normal.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); // Material
		glBufferData(GL_ARRAY_BUFFER, materials.size()*sizeof(Material), materials.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(vao);
		// Vertices
		glEnableVertexAttribArray(attrib_vertex);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		// Normal vectors
		glEnableVertexAttribArray(attrib_normal);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//Material
		for(int i=0; i<6; ++i)
			glEnableVertexAttribArray(attrib_material+i);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glVertexAttribPointer(attrib_material, 3, GL_FLOAT, GL_FALSE, sizeof(Material), nullptr); //Ambient
		glVertexAttribPointer(attrib_material+1, 3, GL_FLOAT, GL_FALSE, sizeof(Material), (void*)offsetof(Material, diffuse)); //Diffuse
		glVertexAttribPointer(attrib_material+2, 3, GL_FLOAT, GL_FALSE, sizeof(Material), (void*)offsetof(Material, specular)); //Specular
		glVertexAttribPointer(attrib_material+3, 1, GL_FLOAT, GL_FALSE, sizeof(Material), (void*)offsetof(Material, shininess)); //shininess
		glVertexAttribPointer(attrib_material+4, 1, GL_FLOAT, GL_FALSE, sizeof(Material), (void*)offsetof(Material, reflectance)); //reflectance
		glVertexAttribPointer(attrib_material+5, 1, GL_FLOAT, GL_FALSE, sizeof(Material), (void*)offsetof(Material, roughness)); //roughness
		// IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		
		// Disable all
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(attrib_vertex);
		glDisableVertexAttribArray(attrib_normal);
		for(int i=0; i<6; ++i)
			glDisableVertexAttribArray(attrib_material+i);
	}
	inline void Draw(){
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

#endif
