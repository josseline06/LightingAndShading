#ifndef _ICOSPHERE_H
#define _ICOSPHERE_H

#include "Definitions.h"

// Model to represent a spoted light source
#define icosphere "models/icosphere.obj"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
using namespace std;

#include <glm\glm.hpp>
using namespace glm;

class Icosphere { // Sphere with radius = 1 
public:
	vector <vec3> vertices;
	vector <unsigned int> indices;
	vec3 maxi, mini;

	Icosphere(){
		FILE* file;
		char buffer[MAX_BUFFER], aux[MAX_SHORT_BUFFER];
		int bytes, bytes_reads;
		unsigned int vindex;
		vec3 vertex;
	
		file = fopen(icosphere, "r");
		
		if (file == NULL) {
			return;
		}
			
		maxi = vec3(-inf);
		mini = vec3(inf);

		while(!feof(file)){
			fscanf(file, "%[^\n]%*[\n]", buffer);
			if(sscanf(buffer, "%s%*[\t\r ]%n", aux, &bytes)<1)
				continue;
			if(strcmp("v", aux)==0){
				sscanf(buffer+bytes, "%f%f%f", &vertex.x, &vertex.y, &vertex.z);
				vertices.push_back(vertex);
				mini = vec3(MIN(mini.x,vertex.x), MIN(mini.y,vertex.y), MIN(mini.z,vertex.z));
				maxi = vec3(MAX(maxi.x,vertex.x), MAX(maxi.y,vertex.y), MAX(maxi.z,vertex.z));
				continue;
			}
			if(strcmp("f", aux)==0){
				for(int i=0; i<3; ++i){
					sscanf(buffer+bytes, "%u%n", &vindex, &bytes_reads);
					vindex--;
					indices.push_back(vindex);
					bytes += bytes_reads;
				}
				continue;
			}
		}
		fclose(file);
	}
};

#endif