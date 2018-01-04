#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <glm\glm.hpp>
using namespace glm;

class Material{
public:
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float reflectance;
	float roughness;
	
	Material(){
		shininess = reflectance = roughness = .5f;
		ambient = diffuse = specular = vec3(0.f);
	}
};

#endif