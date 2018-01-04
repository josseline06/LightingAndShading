# Lighting and Shading

:blue_heart: A C++ app for render simple scenes using OpenGL dynamic pipeline.

## Table of Contents

* [About](#about)
* [System Requirements](#system-requirements)
* [Dependencies](#dependencies)
* [Project Structure](#project-structure)
	* [Metadata](#metadata)
		* [Scene Metadata](#scene-metadata)
		* [Shaders Metadata](#shaders-metadata)
	* [Source Code](#source-code)
* [Graphic User Interface (GUI)](#graphic-user-interface-gui)


## About

The main purpouse of this app is render a simple scene showing several light sources, lighting and shading models using OpenGL dynamic pipeline. This application is available only in a Windows platform. It was developed using C++ as programming language, GLSL for shaders and Visual Studio as IDE, it is a Visual C++ Project.

The scene will be built with OBJ models using materials (MTL file) for reflection parameters. This app doesn't handle textures, it is a very simple app and the development motivation was learn about OpenGL dynamic pipeline.

## System Requirements

* At least Windows 7 SP1 x86 or x64
* OpenGL version >= 2.0
* This project was setup in [Visual Studio 2017 15.3.3](https://www.visualstudio.com/en-us/productinfo/vs2017-system-requirements-vs) but you can open it using Visual Studio 2012 or a higher version

## Dependencies

* Microsoft® C Runtime Library == 14.11 (DLL file was include in `bin/`)

Here the 3rd part libraries used on this project:

* [GLUT](https://www.opengl.org/resources/libraries/glut/) == 3.7.6
* [The OpenGL Extension Wrangler Library (GLEW)](http://glew.sourceforge.net/) >= 1.10
* [AntTweakBar](http://anttweakbar.sourceforge.net/doc/) >= 1.16
* [OpenGL Mathematics (GLM)](https://glm.g-truc.net/0.9.8/index.html) >= 0.9.5

## Project Structure

```shell
LightingAndShading/				# → Root of Visual Studio Project
├── bin/					# → Dependencies' DLL files and the application (.exe)
├── include/					# → Dependencies' headers files (.h, .hpp, etc.)
├── init/					# → Metadata needed for start the app
├── models/					# → All models loaded by the app
├── src/					# → Source code of the app
├── renderings/					# → Captures on one scene from this app
├── LightingAndShading.vcxproj			# → Main project file for VC ++ projects. It stores information specific to each project.
├── LightingAndShading.vcxproj.filters		# → It specifies where to put a file that is added to the solution
└── README.md					# → The read me file. This file :)
```

Here more details for understanding source code:

### Metadata

```shell
LightingAndShading/init     				
├── scene.txt					# → Metadata needed for load all scene's elements
└── shaders.txt					# → Details about shaders. 
```

#### Scene Metadata

Here an explanation about scene.txt file format:

* Line starting by 'E' represents eye position. It must be follow by three floats numbers separated by space `X Y Z`, they represent a 3D position.
* Line starting by 'A' represents ambient light. It must be follow by three floats numbers separated by space `R G B`, they represent a light color. `0 <= R,G,B <= 1`
* For a model the line must be content an 'M' follows by 7 lines:
	1. A string, it is the model name and it can content spaces
	2. A path for a .obj file, it is a string
	3. A path for a .mtl file, it is a string
	4. Three float numbers separate by space `X Y Z`, they represent a scale value per axis. `X,Y,Z > 0.0`  
	5. Three float numbers separate by space `X Y Z`, they represent a rotation value per axis. `-1.0 <= X,Y,Z <= 1.0`
	6. Three float numbers separate by space `X Y Z`, they represent a translation value per axis.
	7. Two integers numbers separate by space `A B`. A represent the lighting model used, it could be 0 (Phong), 1 (Blinn-Phong) or 2 (Cook-Torrance). B represent the shading model used, it could be 0 (Flat), 1 (Gouraud) or 2 (Phong)
* For a ligth source the line must be content an 'L' follows by 3 lines:
	1. Three float numbers separate by space `R G B`, they represent the diffuse component color. `0 <= R,G,B <= 1`
	2. Three float numbers separate by space `R G B`, they represent the specular component color. `0 <= R,G,B <= 1`
	3. An integer number ```N```, it represents the kind of light source. N could be 0 for directional light source, 1 for a positional light and 2 for a spotlight.
	These are aditional parameters, only apply for positional lights and spotlights
	4. Three floats numbers separated by space `C L Q`, they represent constant attenuation, linear attenuation and quadratic attenuation respectively. `0 < C and <= L, Q `
	For a spotlight source is needed the follow parameters:
	5. Three floats numbers separate by space `X Y Z`, they represent a 3D vector for a spot light source direction 
	6. Two integers numbers separate by space `E C`. E represents exponent and C represents cuttoff. `1 <= E, C`

#### Shaders Metadata

Every item in shaders.txt comprises of three lines. These lines represent:
	1. A character, it could be 'S' for shaders used by default or 'L' for shaders with lighting and shading models
	2. A string, it is a path for a vertex shader belong to this item
	3. A string, it is a path for a fragment shader belong to this item

### Source code

```shell
LightingAndShading/src     				
├── shaders/ 				# → All shaders files 
├── Definitions.h 			# → All global variables, flags and macros
├── Icosphere.hpp 			# → It is a class that represent a graphic version of a pointed light source   
├── Light.hpp 				# → Light sources (ambient light no included) class
├── Main.cpp 				# → Main source file, it include set up of all environment and all GUI implementation
├── Material.hpp 				# → Model material class
├── Object.hpp 				# → Model class, include vertices, materials, etc
└── Program.hpp 				# → Shader loading and compilation class
```

## Graphic User Interface (GUI)

When the app starts, it shows your models rendered and lighting and shading initial setup specifies at `init/`. Here a brief explanation about how to use it:

* You can navigate the entire scene getting a first person view. Move your mouse for rotate your perspective and press the arrows from your keyboard (up, down, left, right) and move your position around the scene. 
* You can pick your models from the scene, just click in your model. If you want set the models properties press 'M' and it shows a **Model Menu**. It allows change the shading and lighting models per model, animate it and translate, rotate and scale it. 
* Hide the Model Menu clicking minimize button on the top-right corner or press 'M' again.
* For set the lights sources properties press 'L' and it shows a Lights Menu. This menu allows set the ambient light, and turn on or turn off every light source on this app, set the diffuse and specular component and depending of what kind of light source is it setting its parameters (like direction vector if is directional or its position if is a spotlight, etc.). 
* For hide the **Lights Menu** click minimize button on the top-right corner or press 'L' again.
* If you have any doubt about these options from **Model or Lights Menu**, check **Help and Shortcut section** just clicking the button with a marker symbol (?) at bottom-left corner.
* Press ESC for exit.

P.S.: Thanks to [Pranav Pranchal](https://grabcad.com/pranav.panchal) from [Grabcad Comunity](https://grabcad.com). I took all Angry Birds models from this free library. ***Every model was very helpful!*** 