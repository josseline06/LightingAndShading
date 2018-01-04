#version 400

//Atributos:
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

layout(location = 2) in vec3 ambient;
layout(location = 3) in vec3 diffuse;
layout(location = 4) in vec3 specular;
layout(location = 5) in float shininess;
layout(location = 6) in float reflectance;
layout(location = 7) in float roughness;

uniform mat4 ModelViewProjection;
uniform mat4 ModelView;
uniform mat3 NormalMatrix;

out vec3 Position;
out vec3 Normal;
out vec3 View;

out MaterialInfo{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float reflectance;
	float roughness;
}Material_out;

void main(){
	Position = (ModelView*vec4(vertex, 1.f)).xyz;
	Normal = normalize(NormalMatrix*normal);
	View = normalize(-Position);
	
	Material_out.ambient = ambient;
	Material_out.diffuse = diffuse;
	Material_out.specular = specular;
	Material_out.shininess = shininess;
	Material_out.reflectance = reflectance;
	Material_out.roughness = roughness;
		
	gl_Position = ModelViewProjection*vec4(vertex, 1.f);
}