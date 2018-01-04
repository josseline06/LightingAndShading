#version 400

#define MAX_LIGHTS 3
#define TRUE 1
#define pi 3.14159265358979f

struct Light{
	vec4 position;
	vec3 diffuse;
	vec3 specular;
	float constantAtt; 
	float linearAtt; 
	float quadraticAtt; 
	float cutoff;
	float exponent;
	vec3 direction; 
	int active;
};
uniform Light Lights[MAX_LIGHTS];
uniform vec3 ambient; // Ambient light

uniform mat4 ModelView;
uniform mat3 NormalMatrix;

in MaterialInfo{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
	float reflectance;
	float roughness;
}Material;

in vec3 Normal;
in vec3 Position;
in vec3 View;

vec3 normal, view;

vec3 Intensity(int index){
	vec3 halfv, light;
	vec3 diffuse, specular;
	float att, dist, cos_cutoff, cos_angle, intensity;
	float fresnel, roughness, geometric;
	float NdotH, NdotV, NdotL, HdotV, HdotL; 
	
	att = 1.f;
	if(Lights[index].position.w == 0.f)
		light = normalize(Lights[index].direction);
	else{
		light = Lights[index].position.xyz - Position;
		dist = length(light);
		light = normalize(light);
		att /= Lights[index].constantAtt + Lights[index].linearAtt*dist + Lights[index].quadraticAtt*dist*dist;

		if(Lights[index].cutoff <= 90.f){
			cos_angle = max(dot(-light, normalize(Lights[index].direction)), 0.f);
			cos_cutoff = cos(radians(Lights[index].cutoff));
			att = (cos_angle<cos_cutoff)?0.f:att*pow(cos_angle, Lights[index].exponent);
		}
	}
	halfv = normalize(light+view);
	NdotL = dot(normal, light);
	
	diffuse = specular = vec3(0.f);
	intensity = max(NdotL, 0.f);
	if(intensity>0.f){
		diffuse = Lights[index].diffuse*Material.diffuse*intensity;
		// Calculating specular reflection
		NdotH = dot(normal, halfv);
		NdotV = dot(normal, view);
		HdotV = dot(halfv, view);
		HdotL = dot(halfv, light);
		geometric = min(1, min(2.f*NdotH*NdotV/HdotV, 2.f*NdotH*NdotL/HdotL));
		fresnel = Material.reflectance+(1.f-Material.reflectance)*(1.f-HdotV)*(1.f-HdotV)*(1.f-HdotV)*(1.f-HdotV)*(1.f-HdotV);
		roughness = exp((NdotH*NdotH-1)/(Material.roughness*Material.roughness*NdotH*NdotH))/(pi*Material.roughness*Material.roughness*NdotH*NdotH*NdotH*NdotH);
		intensity = (geometric*fresnel*roughness)/(NdotV*NdotL);
		if(intensity>0.f)
			specular = Lights[index].specular*Material.specular*NdotL*intensity;
	}
	return att*(diffuse+specular);
}

void main(){
	normal = normalize(Normal);
	view = normalize(View);
	
	vec3 color = ambient*Material.ambient;
	for(int i=0; i<MAX_LIGHTS; ++i){
		if(Lights[i].active == TRUE)
			color +=Intensity(i);
	}
	gl_FragColor = vec4(color,1.f);
}