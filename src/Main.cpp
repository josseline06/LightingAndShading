#include "Definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <GL\glew.h>
#include <GL\glut.h>
#include <AntTweakBar.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
using namespace glm;

// --- Custom classes 
#include "Light.hpp"
#include "Program.hpp"
#include "Object.hpp"

// For handle shaders
Lighting l_program[MAX_LIGHTS_PROGRAMS]; // shaders programs
Standar standar;

// Transformations
mat4 view, projection; // scene transformations
mat4 mvp, mv; //ModelViewProjection matrix and ModelView matrix
mat3 normalMatrix; //NormalMatrix

// FPS
vec3 init_position, position; // Eye position
vec3 direction, rightv, up;
float h_angle, v_angle; // horizontal and vertical vision angle

Light lights[MAX_LIGHTS];

Object scene[MAX_MODELS]; // array of models
const vec4 picked_color = vec4(1.f, 0.f, 1.f, 0.4f);

// Window handle
int width, height;
int keyboard_mask, picked, picked_mask; //picked store only position, picked_mask store state
bool fps;
int model_icon, light_icon;

// GUI
TwBar *model_bar, *light_bar;
TwStructMember point3_components[] = {
	{ "X", TW_TYPE_FLOAT, offsetof(vec3, x), "step=0.01" },
	{ "Y", TW_TYPE_FLOAT, offsetof(vec3, y), "step=0.01" },
	{ "Z", TW_TYPE_FLOAT, offsetof(vec3, z), "step=0.01" }
};
TwType point3;

// Bounding box values
GLushort box_indices[] = {
	0, 1, 2, 3,
	4, 5, 6, 7,
	0, 4, 1, 5,
	2, 6, 3, 7
};

/*
Functions section
/*

/*
TemplateBox: create and initialize all buffers and vertex arrays
*/
void TemplateBox(GLuint &vao) {
	GLuint vbo, ibo;

	vec3 vertices[] = {
		// Front side
		vec3(-0.5f, -0.5f, 0.5f),
		vec3(0.5f, -0.5f, 0.5f),
		vec3(0.5f, 0.5f, 0.5f),
		vec3(-0.5f, 0.5f, 0.5f),
		// Back side
		vec3(-0.5f, -0.5f, -0.5f),
		vec3(0.5f, -0.5f, -0.5f),
		vec3(0.5f, 0.5f, -0.5f),
		vec3(-0.5f, 0.5f, -0.5f),
	};
	// Create structures
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(box_indices), box_indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(attrib_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*
DrawBoundingBox: Render a bounding box given an onject from the scene
*/
inline void DrawBoundingBox(GLuint &box_vao) {
	glLineWidth(3.f);
	glBindVertexArray(box_vao);

	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (void*)(sizeof(GLushort) << 2));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (void*)(sizeof(GLushort) << 3));

	glBindVertexArray(0);
	glLineWidth(1.f);
}

/*
LookAt: Calculate normal vector for the scene
*/
inline void LookAt() {
	direction = vec3(sinf(h_angle)*cosf(v_angle), sinf(v_angle), cosf(h_angle)*cosf(v_angle));
	rightv = vec3(-cosf(h_angle), 0.f, sinf(h_angle));
	up = cross(rightv, direction);
}

/*
Start:  Set up the eye to default position
*/
inline void Start() {
	// Assigns default values
	position = init_position;
	h_angle = PI;
	v_angle = 0.f;
	// calculate default normal vector
	LookAt();
}

/*
CreateQuaternion:  Create a new quaternion from rotation axis and an angle
*/
inline quat CreateQuaternion(vec3 axis, float angle) {
	vec3 v = axis*sinf(angle / 2.f);
	float w = cosf(angle / 2.f);
	return quat(w, v);
}

/*
	GLUT callbacks
*/
/*
Reshape: reshape the window and set up the perspective
*/
inline void Reshape(int w, int h) {
	width = (w<1) ? 1 : w;
	height = (h<1) ? 1 : h;

	projection = perspective(radians(FoV), (float)width / (float)height, 0.1f, 100.f);
	glViewport(0, 0, width, height);
	TwWindowSize(width, height);
}

/*
Idle: calculations needed for render every frame
*/
void Idle() {
	static int pt = 0;
	int visible;
	char var[MAX_SIZE_NAME];
	int t = glutGet(GLUT_ELAPSED_TIME);
	float dt = float(t - pt)*0.001f; // in miliseconds

	pt = t; // updating past time  

			// moving around the scene using keyboard navigation
	if (keyboard_mask & UP)
		position += direction*dt*speed;
	if (keyboard_mask & DOWN)
		position -= direction*dt*speed;
	if (keyboard_mask & LEFT)
		position -= rightv*dt*speed;
	if (keyboard_mask & RIGHT)
		position += rightv*dt*speed;

	view = lookAt(position, position + direction, up);

	fps = (!model_icon || !light_icon) ? false : true;
	if (fps)
		glutWarpPointer(width / 2, height / 2);

	//GUI:
	if (!model_icon && (picked_mask & MODEL)) {
		for (int i = 0; i<MAX_MODELS; ++i) {
			visible = (i == picked) ? true : false;
			TwSetParam(model_bar, scene[i].name, "visible", TW_PARAM_INT32, 1, &visible);
		}
	}
	if (!light_icon) {
		for (int i = 0; i<MAX_LIGHTS; ++i) {
			visible = (i == picked || !(picked_mask & LIGHT)) ? true : false;
			sprintf(var, "Light #%d", i + 1);
			TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);
		}
	}
	glutPostRedisplay();
}

/*
DisplaySelection: calculations needed for redering every frame
*/
void DisplaySelection() {
	int r, g, b;

	// Drawing internal scene
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Cleaning 
	glUseProgram(standar.id);
	for (int i = 0; i<MAX_MODELS; ++i) {
		mvp = projection*view*scene[i].ModelMatrix();
		r = (i & 0x000000ff) >> 0;
		g = (i & 0x0000ff00) >> 8;
		b = (i & 0x00ff0000) >> 16;
		standar.SendUniformsToShader(vec4((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.f));
		scene[i].Draw();
	}
	for (int i = 0; i<MAX_LIGHTS; ++i) {
		if (lights[i].position.w == 0)
			continue;
		mvp = projection*view*lights[i].ModelMatrix();
		r = ((i + MAX_MODELS) & 0x000000ff) >> 0;
		g = ((i + MAX_MODELS) & 0x0000ff00) >> 8;
		b = ((i + MAX_MODELS) & 0x00ff0000) >> 16;
		standar.SendUniformsToShader(vec4((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, 1.f));
		lights[i].Draw();
	}
	glUseProgram(0);
	glClearColor(0.f, 0.24f, 0.4f, 1.f); // restore glClearColor
}

void Display() {
	static int pt = 0;
	int t, index;
	float dt, angle;
	vec4 color;
	vec3 axis;

	t = glutGet(GLUT_ELAPSED_TIME);
	dt = float(t - pt)*0.001f; // in miliseconds
	pt = t;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Cleaning 

	// Drawing models
	for (int i = 0; i<MAX_MODELS; ++i) {
		index = 3 * scene[i].lighting + scene[i].shading;
		glUseProgram(l_program[index].id);

		l_program[index].SendUniformsLightsToShader(ambient, ambient_info);
		for (int j = 0; j<MAX_LIGHTS; ++j)
			l_program[index].SendUniformsLightsToShader(lights[j]);

		if (scene[i].animation) {
			angle = scene[i].animation_speed*dt;
			axis = vec3(0.f, 1.f, 0.f);// Y axis
			scene[i].rotation = CreateQuaternion(axis, angle)*scene[i].rotation;
		}

		mv = view*scene[i].ModelMatrix();
		normalMatrix = mat3(transpose(inverse(mv)));
		mvp = projection*mv;
		l_program[index].SendUniformsMatricesToShader();

		scene[i].Draw();
		glUseProgram(0);
	}
	// Drawing lights sources
	glUseProgram(standar.id);
	for (int i = 0; i<MAX_LIGHTS; ++i) {
		if (lights[i].position.w == 0)
			continue;
		mvp = projection*view*lights[i].ModelMatrix();
		color = (lights[i].active) ? lights[i].GetColor() : vec4(vec3(0.f), 1.f);
		standar.SendUniformsToShader(color);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		lights[i].Draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	// 	Drawing bounding box if a model has been selected
	if (!(picked_mask & EMPTY)) {
		if ((picked_mask & MODEL)) { // Object selected
			mvp = projection*view*(scene[picked].ModelMatrix()*scene[picked].box_model);
			standar.SendUniformsToShader(picked_color);
			DrawBoundingBox(model_box_vao);
		}
		else {
			if (lights[picked].position.w == 1.f) { // Light source selected
				mvp = projection*view*lights[picked].ModelMatrix()*box_light_model;
				standar.SendUniformsToShader(picked_color);
				DrawBoundingBox(light_box_vao);
			}
		}
		glUseProgram(0);
	}
	TwDraw();
	glutSwapBuffers();
}

inline void NormalKeys(unsigned char key, int x, int y) {
	if (!TwEventKeyboardGLUT(key, x, y)) {
		switch (key) {
		case 'l':
		case 'L':
			light_icon = !light_icon;
			if (!light_icon && fps)
				fps = false;
			TwSetParam(light_bar, NULL, "iconified", TW_PARAM_INT32, 1, &light_icon);
			break;
		case 'm':
		case 'M':
			if ((picked_mask & MODEL)) {
				model_icon = !model_icon;
				if (!model_icon && fps)
					fps = false;
			}
			else
				model_icon = true;
			TwSetParam(model_bar, NULL, "iconified", TW_PARAM_INT32, 1, &model_icon);
			break;
		case 27: exit(0);
		}
	}
}

inline void SpecialKeys(int key, int x, int y) {
	if (!TwEventSpecialGLUT(key, x, y)) {
		switch (key) {
		case GLUT_KEY_LEFT: keyboard_mask |= LEFT; break;
		case GLUT_KEY_RIGHT: keyboard_mask |= RIGHT; break;
		case GLUT_KEY_UP: keyboard_mask |= UP; break;
		case GLUT_KEY_DOWN: keyboard_mask |= DOWN; break;
		case GLUT_KEY_HOME:
			if (picked_mask & MODEL) // Restore model selected
				scene[picked].Start();
			else { // Restore camera
				Start();
				keyboard_mask = 0;
			}
			break;
		case GLUT_KEY_END: picked_mask = EMPTY;
		}
	}
}

inline void SpecialUpKeys(int key, int x, int y) {
	if (!TwEventSpecialGLUT(key, x, y)) {
		switch (key) {
		case GLUT_KEY_LEFT: keyboard_mask &= ~LEFT; break;
		case GLUT_KEY_RIGHT: keyboard_mask &= ~RIGHT; break;
		case GLUT_KEY_UP: keyboard_mask &= ~UP; break;
		case GLUT_KEY_DOWN: keyboard_mask &= ~DOWN;
		}
	}
}

void Click(int button, int state, int x, int y) {
	if (!TwEventMouseButtonGLUT(button, state, x, y)) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // Calculating picking
			unsigned char data[4];
			DisplaySelection();
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glReadPixels(x, height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			// From color to id
			picked = data[0] + 256 * (data[1] + data[2] * 256);
			if (picked == 0x00ffffff) { // Background
				picked_mask = EMPTY;
				if (!model_icon) {
					model_icon = !model_icon;
					TwSetParam(model_bar, NULL, "iconified", TW_PARAM_INT32, 1, &model_icon);
				}
				if (!light_icon) {
					light_icon = !light_icon;
					TwSetParam(light_bar, NULL, "iconified", TW_PARAM_INT32, 1, &light_icon);
				}
			}
			else {
				if (picked<MAX_MODELS) { // Model
					picked_mask = MODEL;
					if (!light_icon) {
						light_icon = !light_icon;
						TwSetParam(light_bar, NULL, "iconified", TW_PARAM_INT32, 1, &light_icon);
					}
				}
				else { // Light
					picked_mask = LIGHT;
					picked -= MAX_MODELS;
					if (!model_icon) {
						model_icon = !model_icon;
						TwSetParam(model_bar, NULL, "iconified", TW_PARAM_INT32, 1, &model_icon);
					}
				}
			}
		}
	}
}

void Motion(int x, int y) {
	static bool warp = false;
	float dx, dy;

	if (!TwEventMouseMotionGLUT(x, y) && fps) {
		if (!warp) {
			warp = true;
			glutWarpPointer(width / 2, height / 2); //Set up mouse pointer to center

			dx = (float)width / 2.f - (float)x;
			dy = (float)height / 2.f - (float)y;

			h_angle += dx*mouse_speed;
			v_angle += dy*mouse_speed;

			LookAt();
		}
		else
			warp = false;
	}
}

/*
ModelBar: Built all setup options for any model in the scene
*/
void ModelBar() {
	char var[MAX_SIZE_NAME];

	model_bar = TwNewBar("ModelBar");
	TwDefine("ModelBar label='Models Options' size='250 400' alpha=5 color='125 100 215' help = 'Models menu.'");

	TwSetParam(model_bar, NULL, "iconified", TW_PARAM_INT32, 1, &model_icon);

	TwEnumVal shadingT[] = { { flat , "Flat" },{ gouraud, "Gouraud" },{ phong, "Phong" } };
	TwType ShadingModel = TwDefineEnum("ShadingModel", shadingT, 3);

	TwEnumVal lightingT[] = { { _phong , "Phong" },{ blinn_phong, "Blinn-Phong" },{ cook_torrance, "Cook-Torrance" } };
	TwType LightingModel = TwDefineEnum("LightingModel", lightingT, 3);

	TwStructMember animationMembers[] = {
		{ "Active", TW_TYPE_BOOLCPP, offsetof(Object, animation), "help='Animar o no el modelo.'" },
		{ "Speed", TW_TYPE_FLOAT, offsetof(Object, animation_speed), "min=-5 max=5 step=0.1 keyincr=+ keydecr=- help='Model animation speed.'" }
	};
	TwType AnimationType = TwDefineStruct("Animation", animationMembers, 2, sizeof(Object), NULL, NULL);

	TwStructMember MagnitudeMembers[] = {
		{ "X", TW_TYPE_FLOAT, offsetof(vec3, x), "min=0.05 max=3.0 step=0.01" },
		{ "Y", TW_TYPE_FLOAT, offsetof(vec3, y), "min=0.05 max=3.0 step=0.01" },
		{ "Z", TW_TYPE_FLOAT, offsetof(vec3, z), "min=0.05 max=3.0 step=0.01" }
	};
	TwType Magnitude = TwDefineStruct("Magnitude", MagnitudeMembers, 3, sizeof(vec3), NULL, NULL);

	for (int i = 0; i<MAX_MODELS; ++i) {
		sprintf(var, "Shading%d", i);
		TwAddVarRW(model_bar, var, ShadingModel, &scene[i].shading, "label='Shading Model' help='Choose shading model.'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);

		sprintf(var, "Lighting%d", i);
		TwAddVarRW(model_bar, var, LightingModel, &scene[i].lighting, "label='Lighting Model' help='Choose light model.'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);

		sprintf(var, "Rotation%d", i);
		TwAddVarRW(model_bar, var, TW_TYPE_QUAT4F, &scene[i].rotation, "label='Rotation' opened=true help='Model rotation.'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);

		sprintf(var, "Animation%d", i);
		TwAddVarRW(model_bar, var, AnimationType, &scene[i], "label='Animation' opened=true help='Model animation options.'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);

		sprintf(var, "Zoom%d", i);
		TwAddVarRW(model_bar, var, Magnitude, &scene[i].scaling, "label='Zoom' help='Model zoom per axis (1 => 100%).'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);

		sprintf(var, "PositionM%d", i);
		TwAddVarRW(model_bar, var, point3, &scene[i].translation, "label='Position' help='Model position'");
		TwSetParam(model_bar, var, "group", TW_PARAM_CSTRING, 1, scene[i].name);
	}
}
/*
LightsBar Callbacks
*/
void TW_CALL SetTypeLightCB(const void *value, void *client_data) {
	int val, visible;
	char var[MAX_SIZE_NAME];
	Light *selected = (static_cast<Light *>(client_data));

	val = *(int *)value;

	if ((selected->type == spotlight) && (selected->last_cutoff != selected->cutoff))
		selected->last_cutoff = selected->cutoff;

	if (selected->type == val)
		return;
	selected->type = val;

	sprintf(var, "Direction%d", selected->id);
	visible = selected->type == directional;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	sprintf(var, "PositionL%d", selected->id);
	visible = selected->type != directional;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	sprintf(var, "Attenuation%d", selected->id);
	visible = selected->type != directional;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	sprintf(var, "Spot_Direction%d", selected->id);
	visible = selected->type == spotlight;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	sprintf(var, "Exponent%d", selected->id);
	visible = selected->type == spotlight;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	sprintf(var, "Cutoff%d", selected->id);
	visible = selected->type == spotlight;
	TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

	selected->cutoff = (selected->type == spotlight) ? selected->last_cutoff : 180.f;
	selected->position.w = (selected->type == directional) ? 0.f : 1.f;
}

void TW_CALL GetTypeLightCB(void *value, void *client_data) {
	*static_cast <int *>(value) = static_cast<Light *>(client_data)->type;
}

void TW_CALL SetDirectionCB(const void *value, void *client_data) {
	Light *selected = (static_cast<Light *>(client_data));
	vec3 val = *(vec3 *)value;
	selected->direction = -val;

}

void TW_CALL GetDirectionCB(void *value, void *client_data) {
	*static_cast <vec3 *>(value) = -vec3(static_cast<Light *>(client_data)->direction);
}

void TW_CALL SetSpotDirectionCB(const void *value, void *client_data) {
	Light *selected = (static_cast<Light *>(client_data));
	vec3 val = *(vec3 *)value;
	selected->direction = val;
}

void TW_CALL GetSpotDirectionCB(void *value, void *client_data) {
	*static_cast <vec3 *>(value) = static_cast<Light *>(client_data)->direction;
}

/*
LightBar: Built all setup options for any light source (including global ambient light)
*/
void LightBar() {
	char group[MAX_SIZE_NAME], var[MAX_SIZE_NAME];
	int visible;

	light_bar = TwNewBar("LightBar");
	TwDefine("LightBar label='Lights Options' position='758 16' size='250 400' alpha=5 color='125 100 215' help = 'Light sources menu.'");

	TwSetParam(light_bar, NULL, "iconified", TW_PARAM_INT32, 1, &light_icon);

	// Setting up all choice fields
	TwEnumVal type_light[] = { { directional , "Directional" },{ spot, "Spot" },{ spotlight, "Spotlight" } };
	TwType TypeLight = TwDefineEnum("TypeLight", type_light, 3);

	TwStructMember AttenuationMembers[] = {
		{ "Constant", TW_TYPE_FLOAT, offsetof(Light, constantAtt), "min=0.1 max=2.1 step=0.01 help='Light constant attenuation.'" },
		{ "Linear", TW_TYPE_FLOAT, offsetof(Light, linearAtt), "min=0 max=2 step=0.01 help='Light linear attenuation.'" },
		{ "Quadratic", TW_TYPE_FLOAT, offsetof(Light, quadraticAtt), "min=0 max=2 step=0.01 help='Light quadratic attenuation.'" }
	};
	TwType AttenuationType = TwDefineStruct("AttenuationType", AttenuationMembers, 3, sizeof(Light), NULL, NULL);

	TwStructMember LightComponentsMembers[] = {
		{ "Diffuse", TW_TYPE_COLOR3F, offsetof(Light, diffuse), "help='Light diffuse component.'" },
		{ "Specular", TW_TYPE_COLOR3F, offsetof(Light, specular), "help='Light specular component.'" }
	};
	TwType LightComponents = TwDefineStruct("LightComponents", LightComponentsMembers, 2, sizeof(Light), NULL, NULL);

	// Ambient Light
	TwAddVarRW(light_bar, "Ambient", TW_TYPE_COLOR3F, &ambient, "label='Ambient Light' help='Choose a color for your ambient light'");
	TwAddSeparator(light_bar, "sep_1", NULL);

	// Options per light source
	for (int i = 0; i<MAX_LIGHTS; ++i) {
		sprintf(group, "Light #%d", i + 1);

		lights[i].type = (lights[i].position.w == 0) ? directional : ((lights[i].cutoff <= 90.f) ? spotlight : spot);
		lights[i].id = i;

		sprintf(var, "Active%d", i);
		TwAddVarRW(light_bar, var, TW_TYPE_BOOLCPP, &lights[i].active, "label='Active' help='Model animation menu.'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);

		sprintf(var, "Components%d", i);
		TwAddVarRW(light_bar, var, LightComponents, &lights[i], "label='Components' help='Light components.'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);

		sprintf(var, "TypeLight%d", i);
		TwAddVarCB(light_bar, var, TypeLight, SetTypeLightCB, GetTypeLightCB, &lights[i], "label='Type' help='Kinds of lights sources.'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);

		sprintf(var, "Direction%d", i);
		TwAddVarCB(light_bar, var, TW_TYPE_DIR3F, SetDirectionCB, GetDirectionCB, &lights[i], "label='Direction' help='Light direction.' arrowcolor='0 255 255' showval=false");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type == directional;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

		sprintf(var, "PositionL%d", i);
		TwAddVarRW(light_bar, var, point3, &lights[i].position, "label='Position' help='Light position'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type != directional;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

		sprintf(var, "Attenuation%d", i);
		TwAddVarRW(light_bar, var, AttenuationType, &lights[i], "label='Attenuation' help='Attenuation intensity.'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type != directional;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

		sprintf(var, "Spot_Direction%d", i);
		TwAddVarCB(light_bar, var, TW_TYPE_DIR3F, SetSpotDirectionCB, GetSpotDirectionCB, &lights[i], "label='Spot Direction' help='Direction vector from the spot light source.' arrowcolor='0 255 255' showval=false");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type == spotlight;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

		sprintf(var, "Exponent%d", i);
		TwAddVarRW(light_bar, var, TW_TYPE_FLOAT, &lights[i].exponent, "label='Exponent' min=1 max=128 step=1.0 help='Focus light intensity.'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type == spotlight;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);

		sprintf(var, "Cutoff%d", i);
		TwAddVarRW(light_bar, var, TW_TYPE_FLOAT, &lights[i].cutoff, "label='Cutoff' min=1 max=90 step=1.0 help='Focal aperture (in degrees).'");
		TwSetParam(light_bar, var, "group", TW_PARAM_CSTRING, 1, group);
		visible = lights[i].type == spotlight;
		TwSetParam(light_bar, var, "visible", TW_PARAM_INT32, 1, &visible);
	}

	TwAddVarRO(light_bar, "camera", point3, &position, "label='Camera'");
}

/*
InitOpenGL: Enable OpenGL flags and load metadata for scene and shaders
*/
bool InitOpenGL() {
	FILE *file;
	int i, j;
	char op, path_1[_MAX_PATH], path_2[_MAX_PATH];

	glClearColor(0.f, 0.24f, 0.4f, 1.f);

	// Enabling z-buffer
	glEnable(GL_DEPTH_TEST);

	// Enabling back-face culling
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// Blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION); // Normal vector per polygon

	TemplateLight();

	// Reading shaders' metadata
	file = fopen("init/shaders.txt", "r");
	if (file) {
		i = 0;
		printf("INFO: Loading Shaders Metadata\n");
		while (!feof(file)) {
			fscanf(file, "%c%s%s%*[ \n\t]", &op, path_1, path_2);
			if (op == 'S') { // standar shader
				standar.Create(path_1, path_2);
				standar.GetUniforms();
			}
			else { // lighting shader
				l_program[i].Create(path_1, path_2);
				l_program[i].GetUniforms();
				++i;
			}
		}
		printf("INFO: Shaders Metadata loaded!\n");
		fclose(file);
	}else {
		fprintf(stderr, "ERROR: Shaders Metadata file not found\n");
		return false;
	}
		
	// Reading scene's metadata
	file = fopen("init/scene.txt", "r");
	if (file) {
		i = 0;
		j = 0;
		printf("INFO: Loading Scene Metadata\n");
		while (!feof(file)) {
			fscanf(file, "%c%*[ \n\t]", &op);
			switch (op) {
			case 'A':
				printf("INFO: Loading ambient light...\n");
				fscanf(file, "%f%f%f%*[ \n\t]", &ambient.x, &ambient.y, &ambient.z);
				continue;
			case 'E':
				printf("INFO: Loading eye position...\n");
				fscanf(file, "%f%f%f%*[ \n\t]", &init_position.x, &init_position.y, &init_position.z);
				continue;
			case 'M':
				fscanf(file, "%[^\n\t]%*[\n\t]", scene[i].name);
				printf("INFO: Loading model: %s...\n", scene[i].name);
				fscanf(file, "%[^\n\t]%*[\n\t]", path_1);
				fscanf(file, "%[^\n\t]%*[\n\t]", path_2);
				fscanf(file, "%f%f%f%*[ \n\t]", &scene[i].init_s.x, &scene[i].init_s.y, &scene[i].init_s.z);
				fscanf(file, "%f%f%f%f%*[ \n\t]", &scene[i].init_r.x, &scene[i].init_r.y, &scene[i].init_r.z, &scene[i].init_r.w);
				fscanf(file, "%f%f%f%*[ \n\t]", &scene[i].init_t.x, &scene[i].init_t.y, &scene[i].init_t.z);
				fscanf(file, "%d%d%*[ \n\t]", &scene[i].lighting, &scene[i].shading);
				scene[i].Init(path_1, path_2);
				++i;
				continue;
			case 'L':
				lights[j].active = true;
				printf("INFO: Loading light source #%d...\n", j + 1);
				fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].diffuse.r, &lights[j].diffuse.g, &lights[j].diffuse.b);
				fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].specular.r, &lights[j].specular.g, &lights[j].specular.b);
				fscanf(file, "%d%*[ \n\t]", &lights[j].type);
				if (lights[j].type == directional) {
					fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].direction.x, &lights[j].direction.y, &lights[j].direction.z);
					continue;
				}
				fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].position.x, &lights[j].position.y, &lights[j].position.z);
				lights[j].position.w = 1.f;
				fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].constantAtt, &lights[j].linearAtt, &lights[j].quadraticAtt);
				if (lights[j].type == spot)
					continue;
				fscanf(file, "%f%f%f%*[ \n\t]", &lights[j].direction.x, &lights[j].direction.y, &lights[j].direction.z);
				fscanf(file, "%f%f%*[ \n\t]", &lights[j].exponent, &lights[j].cutoff);
				++j;
			}
		}
		printf("INFO: Scene Metadata loaded!\n");
		fclose(file);
	}else {
		fprintf(stderr, "ERROR: Scene Metadata file not found\n");
		return false;
	}
		
	// Initialize camera
	Start();

	// Bounding box
	TemplateBox(model_box_vao); // For models
	TemplateBox(light_box_vao); // For lights

	// GUI flags
	model_icon = light_icon = true;
	fps = true;
	picked_mask = EMPTY;
	keyboard_mask = picked = 0;

	// Center mouse's pointer 
	glutWarpPointer(width / 2, height / 2);

	//GUI: Global parameters
	TwDefine("GLOBAL fontSize=3 help = '3D Scene Lighting and Shading.' contained=true");
	point3 = TwDefineStruct("point3", point3_components, 3, sizeof(vec3), NULL, NULL);

	return true;
}

/*
main: Call all set up functions and running glut
*/
int main(int argv, char *argc) {
	char root[_MAX_PATH] = "";

	GLint status;
	width = 1024;
	height = 700;

	printf("------------------------------\nSystem Information\n------------------------------\n");
	// Getting root directory
	GetCurrentDirectoryA(_MAX_PATH, root);
	int offset = strlen(root) - 3; // |"bin"| = 3
	if (strcmp(root + offset, "bin") == 0) {
		// In case of execute in bin/, parent directory must be root directory 
		root[offset] = '\0';
		SetCurrentDirectoryA(root);
	}
	GetCurrentDirectoryA(_MAX_PATH, root);
	printf("Your current directory is %s\n", root);

	// Init GLUT
	glutInit(&argv, &argc);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);

	glutCreateWindow("Lighting and Shading");

	status = glewInit();
	if (status != GLEW_OK) {
		fprintf(stderr, "Error %s \n", glewGetErrorString(status));
		system("pause");
		return 0;
	}
	printf("Vendor : %s\n", glGetString(GL_VENDOR));
	printf("Renderer : %s\n", glGetString(GL_RENDERER));
	printf("OpenGL Version : %s\n", glGetString(GL_VERSION));
	printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("------------------------------\n"); // System information ends

	// Only run if the graphic card supports OpenGL 2.0 version and higher
	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "ERROR: Your graphic card doesn't support OpenGL 2.0\n");
		system("pause");
		return 0;
	}

	// Init AntTweakBar
	TwInit(TW_OPENGL, NULL);

	// Init all structures related to OpenGL
	if (!InitOpenGL()) {
		system("pause");
		return 0;
	}

	// Loading GLUT callbacks
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);
	glutMotionFunc(Motion);
	glutPassiveMotionFunc(Motion);
	glutMouseFunc(Click);
	glutKeyboardFunc(NormalKeys);
	glutSpecialFunc(SpecialKeys);
	glutSpecialUpFunc(SpecialUpKeys);
	glutDisplayFunc(Display);
	
	//Loading GUI
	ModelBar();
	LightBar();

	glutMainLoop();

	return 0;
}