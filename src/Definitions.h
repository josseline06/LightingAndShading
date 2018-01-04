// Disable warning messages 4996 and 4804
#pragma warning(disable:4996 4804)

// Constants related to load
#ifndef MAX_BUFFER
#define MAX_BUFFER 1000
#endif 

#ifndef MAX_SHORT_BUFFER
#define MAX_SHORT_BUFFER 100
#endif 

#ifndef MAX_SIZE_NAME
#define MAX_SIZE_NAME 30
#endif 

// Constants related to lights
#ifndef MAX_LIGHTS_PROGRAMS
#define MAX_LIGHTS_PROGRAMS 9
#endif 

#ifndef MAX_LIGHTS
#define MAX_LIGHTS 3
#endif 

// You can load up to MAX_MODELS value on this program
#ifndef MAX_MODELS
#define MAX_MODELS 12
#endif 

// Optional parameters to shaders
#ifndef attrib_vertex
#define attrib_vertex 0
#endif

#ifndef attrib_normal
#define attrib_normal 1
#endif

#ifndef attrib_material
#define attrib_material 2
#endif

// Math constants
#ifndef inf
#define inf 0x3f3f3f
#endif

#ifndef PI
#define PI 3.14159265358979f
#endif

#ifndef FoV
#define FoV 60.f
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif 

// Animation constants
#ifndef speed
#define speed 1.5f
#endif 

#ifndef mouse_speed
#define mouse_speed 0.003f
#endif 

// Keyboard navigation constants
#ifndef UP
#define UP 1
#endif 

#ifndef DOWN
#define DOWN 2
#endif 

#ifndef LEFT
#define LEFT 4
#endif 

#ifndef RIGHT
#define RIGHT 8
#endif 

// Lighting models
#ifndef _phong
#define _phong 0
#endif 

#ifndef blinn_phong
#define blinn_phong 1
#endif 

#ifndef cook_torrance
#define cook_torrance 2
#endif 

// Shading models
#ifndef flat
#define flat 0
#endif 

#ifndef gouraud
#define gouraud 1
#endif 

#ifndef phong
#define phong 2
#endif 

// Light sources
#ifndef directional
#define directional 0
#endif 

#ifndef spot
#define spot 1
#endif 

#ifndef spotlight
#define spotlight 2
#endif 

// Pixels belongs to
#ifndef EMPTY
#define EMPTY 1
#endif 

#ifndef MODEL
#define MODEL 2
#endif 

#ifndef LIGHT
#define LIGHT 4
#endif 

// --- Custom macros 
/*
MAX: Return maximum between two ordinals elements
*/
#ifndef MAX
#define MAX(a, b) ((a)>(b))?(a):(b)
#endif

/*
MIN: Return minimun between two ordinals elements
*/
#ifndef MIN
#define MIN(a, b) ((a)<(b))?(a):(b)
#endif

/*
INFINITY_NORM: Return the maximum between components of a 3D vector
*/
#ifndef INFINITY_NORM
#define INFINITY_NORM(a, b, c) MAX(MAX((a),(b)), (c))
#endif