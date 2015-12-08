//How to run?

Just run ./DoIt.sh

it will extract dragon2.ply, delete the existing lab5 executable and compile a new one.

I use my own dragon2.ply because I generated the nx,ny,nz for it. Please use my version of PLYs! Same goes to bunny2.ply

//What have I done?

* PLY models
	- read/draw PLY model
	- VBO
	- shaders

* Texture mapping  
	- drew a box placing the PLYs
	- uses GLSL, not fixed function pipeline

* Cube Mapping
	- 6 walls
	- cube mapping
	- Reflections of environment on PLY models

* Submit image
	- open ./lab5.png