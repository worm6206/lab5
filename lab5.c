
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <stack> 
#include <math.h> 
#include <string> 
#include "ply.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std; 

#ifdef __APPLE__
#include <GL/glew.h> 
#include <GLUT/glut.h> 
#include <OpenGL/gl.h> 
#else 
#include <GL/glew.h> 
#include <GL/glut.h> 
#include <GL/gl.h>
#define M_PI 3.1415926
#endif
#define GLM_FORCE_RADIANS


#include<glm/glm.hpp> 
#include<glm/gtx/transform.hpp>

GLuint programObject;
GLuint SetupGLSL(char*); 

typedef struct 
{
  float location[4]; 
  float normal[4]; 
  float color[4]; 
  float tex[3];
} VertexWithTex; 


typedef struct {
  float x, y, z;
  float nx, ny, nz;
} Vertex;

typedef struct Face {
  unsigned int count;
  unsigned int *vertices;
  float nx, ny, nz;
} Face;

char* string_list[] = {
  "x", "y", "z", "nx", "ny", "nz", "vertex_indices"
};
////PLY1
Vertex** vertices = 0;
Face** faces = 0;
unsigned int vertexcount;
unsigned int facecount;
int vertexnormals = 0;
int facenormals = 0;
///PLY2
Vertex** vertices2 = 0;
Face** faces2 = 0;
unsigned int vertexcount2;
unsigned int facecount2;
int vertexnormals2 = 0;
int facenormals2 = 0;

GLuint vboHandle[3];   
GLuint indexVBO[3];

GLubyte readImage[400][400][4]; 
GLubyte texImage[256][256][4]; 

float angle1=0, angle2=0;
float angle3=0, angle4=0;

glm::mat4 modelM = glm::mat4(1.0f); 

int texture =0;
stack<glm::mat4> mat_stack;


GLfloat light_ambient[4] = {0.8,0.8,0.8,1};  //Ia 
GLfloat light_diffuse[4] = {0.8,0.8,0.8,1};  //Id
GLfloat light_specular[4] = {1,1,1,1};  //Is
GLfloat light_pos [4] = {4, 4, 1.8, 1};


GLfloat mat_ambient[4] = {0.5,0.5,0.5,1};  //Ka 
GLfloat mat_diffuse[4] = {0.8,0.8,0,1};  //Kd
GLfloat mat_specular[4] = {1,1,1,1};  //Ks
GLfloat mat_shine[1] = {10}; 

int xform_mode = 0; 
#define XFORM_NONE    0 
#define XFORM_ROTATE  1
#define XFORM_SCALE 2 

int press_x, press_y; 
int release_x, release_y; 
float z_angle = 0.0;
float x_angle = 0.0; 
float scale_size = 1;

bool WIRE_FRAME =false; 

void store_ply(PlyFile* input, Vertex ***vertices, Face ***faces,
	       unsigned int* vertexcount, unsigned int* facecount,
	       int* vertexnormals, int* facenormals) {
  int i, j;

  // go through the element types
  for(i = 0; i < input->num_elem_types; i = i + 1) {
    int count;
    
    // setup the element for reading and get the element count
    char* element = setup_element_read_ply(input, i, &count);

    // get vertices
    if(strcmp("vertex", element) == 0) {
      *vertices = (Vertex**)malloc(sizeof(Vertex) * count);
      *vertexcount = count;

      // run through the properties and store them
      for(j = 0; j < input->elems[i]->nprops; j = j + 1) {
	PlyProperty* property = input->elems[i]->props[j];
	PlyProperty setup;

	if(strcmp("x", property->name) == 0 &&
	   property->is_list == PLY_SCALAR) {

	  setup.name = string_list[0];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, x);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	}
	else if(strcmp("y", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[1];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, y);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	}
	else if(strcmp("z", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[2];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, z);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	}
	else if(strcmp("nx", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[3];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, nx);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *vertexnormals = 1;
	}
	else if(strcmp("ny", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[4];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, ny);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *vertexnormals = 1;
	}
	else if(strcmp("nz", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[5];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Vertex, nz);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *vertexnormals = 1;
	}
	// dunno what it is
	else {
	  fprintf(stderr, "unknown property type found in %s: %s\n",
		  element, property->name);
	}
      }

      // do this if you want to grab the other data
      // list_pointer = get_other_properties_ply
      //                (input, offsetof(Vertex, struct_pointer));

      // copy the data
      for(j = 0; j < count; j = j + 1) {
	(*vertices)[j] = (Vertex*)malloc(sizeof(Vertex));
	
	get_element_ply(input, (void*)((*vertices)[j]));
      }
    }
    // get faces
    else if(strcmp("face", element) == 0) {
      *faces = (Face**)malloc(sizeof(Face) * count);
      *facecount = count;

      // run through the properties and store them
      for(j = 0; j < input->elems[i]->nprops; j = j + 1) {
	PlyProperty* property = input->elems[i]->props[j];
	PlyProperty setup;

	if(strcmp("vertex_indices", property->name) == 0 &&
	   property->is_list == PLY_LIST) {

	  setup.name = string_list[6];
	  setup.internal_type = Uint32;
	  setup.offset = offsetof(Face, vertices);
	  setup.count_internal = Uint32;
	  setup.count_offset = offsetof(Face, count);

	  setup_property_ply(input, &setup);
	}
	else if(strcmp("nx", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[3];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Face, nx);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *facenormals = 1;
	}
	else if(strcmp("ny", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[4];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Face, ny);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *facenormals = 1;
	}
	else if(strcmp("nz", property->name) == 0 &&
		property->is_list == PLY_SCALAR) {

	  setup.name = string_list[5];
	  setup.internal_type = Float32;
	  setup.offset = offsetof(Face, nz);
	  setup.count_internal = 0;
	  setup.count_offset = 0;

	  setup_property_ply(input, &setup);
	  *facenormals = 1;
	}
	// dunno what it is
	else {
	  fprintf(stderr, "unknown property type found in %s: %s\n",
		  element, property->name);
	}
      }
	
      // do this if you want to grab the other data
      // list_pointer = get_other_properties_ply
      //                (input, offsetof(Face, struct_pointer));
      
      // copy the data
      for(j = 0; j < count; j = j + 1) {
	(*faces)[j] = (Face*)malloc(sizeof(Face));
	
	get_element_ply(input, (void*)((*faces)[j]));
      }
    }
    // who knows?
    else {
      fprintf(stderr, "unknown element type found: %s\n", element);
    }
  }

  // if you want to grab the other data do this
  // get_other_element_ply(input);
}


void read_Image() 
{
  FILE* in = fopen("buck.ppm", "r"); 

  int height, width, ccv; 
  char header[100]; 
  fscanf(in, "%s %d %d %d", header, &width, &height, &ccv); 

  printf("%s %d %d %d\n", header, width, height, ccv);
  int r, g, b; 

  for (int i=height-1; i>=0; i--)
     for (int j=0; j<width; j++)
{
      fscanf(in, "%d %d %d", &r, &g, &b); 
      readImage[i][j][0] = (GLubyte)r; 
      readImage[i][j][1] = (GLubyte)g; 
      readImage[i][j][2] = (GLubyte )b; 
      readImage[i][j][3] = 255; 
    }

  for (int i=0; i<256; i++)
    for ( int j=0; j<256; j++) {
      if (i<height && j <width) {
	texImage[i][j][0] = readImage[i][j][0]; 
	texImage[i][j][1] = readImage[i][j][1];
	texImage[i][j][2] = readImage[i][j][2];
	texImage[i][j][3] = 255; 
      }
      else {
      	texImage[i][j][0] = 0; 
	texImage[i][j][1] = 0; 
	texImage[i][j][2] = 0; 
	texImage[i][j][3] = 255; 
      }
    }
  
  fclose(in); 
}

bool load_cube_map_side (
  GLuint texture, GLenum side_target, const char* file_name
) {
  glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

  int x, y, n;
  int force_channels = 4;
  unsigned char*  image_data = stbi_load (
    file_name, &x, &y, &n, force_channels);
  if (!image_data) {
    fprintf (stderr, "ERROR: could not load %s\n", file_name);
    return false;
  }
  // non-power-of-2 dimensions check
  if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
    fprintf (
      stderr, "WARNING: image %s is not power-of-2 dimensions\n", file_name
    );
  }
  
  // copy image data into 'target' side of cube map
  glTexImage2D (side_target,0,GL_RGBA,x,y,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
  free (image_data);
  return true;
}

GLuint cube_tex; 

void MapVBO(){
	// glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures (1, &cube_tex);
	glActiveTexture (GL_TEXTURE0);
	cout << cube_tex << endl;
	// glBindTexture (GL_TEXTURE_CUBE_MAP, cube_tex);

	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "negz.jpg"));
	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "posz.jpg"));
	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "posy.jpg"));
	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "negy.jpg"));
	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "negx.jpg"));
	assert (load_cube_map_side (cube_tex, GL_TEXTURE_CUBE_MAP_POSITIVE_X, "posx.jpg"));
	// for (int i = 0; i < 6; i++)
	// {
 //  	glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,texImage);
	// }

	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


}


GLuint cube_tex2; 

void MapVBO2(){
	// glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures (1, &cube_tex2);
	glActiveTexture (GL_TEXTURE1);
	cout << cube_tex2 << endl;
	// glBindTexture (GL_TEXTURE_CUBE_MAP, cube_tex);

	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "O.jpg"));
	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "O.jpg"));
	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "O.jpg"));
	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "O.jpg"));
	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "O.jpg"));
	assert (load_cube_map_side (cube_tex2, GL_TEXTURE_CUBE_MAP_POSITIVE_X, "O.jpg"));
	// for (int i = 0; i < 6; i++)
	// {
 //  	glTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,texImage);
	// }

	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


}

void INTtexture(){
	// glEnable(GL_TEXTURE_CUBE_MAP);
	// GLuint renderTex;
	GLuint gradientTex;
	read_Image();
	// glGenTextures(1,&renderTex);
	// glActiveTexture(GL_TEXTURE0);
	// glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,texImage);


	// glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glGenTextures(1,&gradientTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, gradientTex);
	glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,texImage);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



}


VertexWithTex SQ[4];
GLubyte SQindices[6];
	// SQ = new VertexWithTex[4];

void SQvalues(){
	//location
	SQ[0].location[0] = -0.5;
	SQ[0].location[1] = -0.5;
	SQ[0].location[2] = 0;

	SQ[1].location[0] = 0.5;
	SQ[1].location[1] = -0.5;
	SQ[1].location[2] = 0;

	SQ[2].location[0] = 0.5;
	SQ[2].location[1] = 0.5;
	SQ[2].location[2] = 0;

	SQ[3].location[0] = -0.5;
	SQ[3].location[1] = 0.5;
	SQ[3].location[2] = 0;

	SQ[0].location[3] = SQ[1].location[3] = SQ[2].location[3] = SQ[3].location[3] = 1;
	
	//normal
	SQ[0].normal[0] = 0;
	SQ[0].normal[1] = 0;
	SQ[0].normal[2] = 1;

	SQ[1].normal[0] = 0;
	SQ[1].normal[1] = 0;
	SQ[1].normal[2] = 1;

	SQ[2].normal[0] = 0;
	SQ[2].normal[1] = 0;
	SQ[2].normal[2] = 1;

	SQ[3].normal[0] = 0;
	SQ[3].normal[1] = 0;
	SQ[3].normal[2] = 1;

	SQ[0].normal[3] = SQ[1].normal[3] = SQ[2].normal[3] = SQ[3].normal[3] = 0;

	//tex
	SQ[0].tex[0] = -1;
	SQ[0].tex[1] = -1;

	SQ[1].tex[0] = 1;
	SQ[1].tex[1] = -1;

	SQ[2].tex[0] = 1;
	SQ[2].tex[1] = 1;

	SQ[3].tex[0] = -1;
	SQ[3].tex[1] = 1;

	SQ[0].tex[2] =SQ[1].tex[2] =SQ[2].tex[2] =SQ[3].tex[2] = 1;

	//order
	SQindices[0] = 0;
	SQindices[1] = 1;
	SQindices[2] = 2;
	SQindices[3] = 0;
	SQindices[4] = 2;
	SQindices[5] = 3;
}

VertexWithTex *cubedata;
GLubyte tindices[36]; 

void cubeValue(){

  /*
  So this part is about defining cube's endpoints and their normals.
  I thought that one point can be included in 3 planes, which are facing different axis, thus should have
  differnt normal. So below is what it's about. The result is better than professor Shen's implementation,
  but not what I expected to be, didn't fix it due to time constrain.
  */
  //6planes * 4points form a plane = 24
  cubedata = new VertexWithTex[24];


  cubedata[0].location[0] = cubedata[4].location[0] = cubedata[21].location[0] = -0.5; cubedata[0].location[1] = cubedata[4].location[1] = cubedata[21].location[1] = -0.5; 
  cubedata[0].location[2] = cubedata[4].location[2] = cubedata[21].location[2] = -0.5; cubedata[0].location[3] = cubedata[4].location[3] = cubedata[21].location[3] = 1.0;

  cubedata[1].location[0] = cubedata[5].location[0] =  cubedata[17].location[0] = 0.5; cubedata[1].location[1] = cubedata[5].location[1] = cubedata[17].location[1] = -0.5;
  cubedata[1].location[2] = cubedata[5].location[2] = cubedata[17].location[2] = -0.5; cubedata[1].location[3] = cubedata[5].location[3] = cubedata[17].location[3] =  1.0;

  cubedata[2].location[0] =  cubedata[8].location[0] =  cubedata[18].location[0] =  0.5; cubedata[2].location[1] =  cubedata[8].location[1] =  cubedata[18].location[1] =   0.5;
  cubedata[2].location[2] = cubedata[8].location[2] = cubedata[18].location[2] = -0.5; cubedata[2].location[3] =  cubedata[8].location[3] =  cubedata[18].location[3] =  1.0;

  cubedata[3].location[0] = cubedata[9].location[0] = cubedata[22].location[0] = -0.5; cubedata[3].location[1] =  cubedata[9].location[1] =  cubedata[22].location[1] =  0.5;
  cubedata[3].location[2] = cubedata[9].location[2] = cubedata[22].location[2] = -0.5; cubedata[3].location[3] = cubedata[9].location[3] = cubedata[22].location[3] =  1.0;

  cubedata[12].location[0] = cubedata[7].location[0] = cubedata[20].location[0] = -0.5; cubedata[12].location[1] = cubedata[7].location[1] = cubedata[20].location[1] = -0.5;
  cubedata[12].location[2] =  cubedata[7].location[2] =  cubedata[20].location[2] =  0.5; cubedata[12].location[3] = cubedata[7].location[3] = cubedata[20].location[3] =  1.0;

  cubedata[13].location[0] =  cubedata[6].location[0] = cubedata[16].location[0] =  0.5; cubedata[13].location[1] = cubedata[6].location[1] = cubedata[16].location[1] = -0.5;
  cubedata[13].location[2] = cubedata[6].location[2] = cubedata[16].location[2] =  0.5; cubedata[13].location[3] = cubedata[6].location[3] = cubedata[16].location[3] =  1.0;

  cubedata[14].location[0] = cubedata[11].location[0] = cubedata[19].location[0] =  0.5; cubedata[14].location[1] = cubedata[11].location[1] = cubedata[19].location[1] =  0.5;
  cubedata[14].location[2] =  cubedata[11].location[2] =  cubedata[19].location[2] =  0.5; cubedata[14].location[3] = cubedata[11].location[3] = cubedata[19].location[3] =  1.0;

  cubedata[15].location[0] = cubedata[10].location[0] = cubedata[23].location[0] = -0.5; cubedata[15].location[1] = cubedata[10].location[1] = cubedata[23].location[1] =  0.5;
  cubedata[15].location[2] = cubedata[10].location[2] = cubedata[23].location[2] =  0.5; cubedata[15].location[3] = cubedata[10].location[3] = cubedata[23].location[3] =  1.0;



  int xx=0;
  //facing -z
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = 0.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = 0.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = -1.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
  xx+=4;
  //facing -y
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = 0.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = -1.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = 0.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
  xx+=4;
  //facing y
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = 0.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = 1.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = 0.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
  xx+=4;
  //facing z
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = 0.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = 0.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = 1.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
  xx+=4;
  //facing x
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = 1.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = 0.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = 0.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
  xx+=4;
  //facing -x
  cubedata[xx].normal[0] = cubedata[xx+1].normal[0] = cubedata[xx+2].normal[0] = cubedata[xx+3].normal[0] = -1.0; 
  cubedata[xx].normal[1] = cubedata[xx+1].normal[1] = cubedata[xx+2].normal[1] = cubedata[xx+3].normal[1] = 0.0; 
  cubedata[xx].normal[2] = cubedata[xx+1].normal[2] = cubedata[xx+2].normal[2] = cubedata[xx+3].normal[2] = 0.0; 
  cubedata[xx].normal[3] = cubedata[xx+1].normal[3] = cubedata[xx+2].normal[3] = cubedata[xx+3].normal[3] = 1.0;
    //0~23
  int yy=0; int zz=0;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;
  yy+=4;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;
  yy+=4;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;
  yy+=4;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;
  yy+=4;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;
  yy+=4;
  tindices[zz++] = yy;   tindices[zz++] = yy+1;   tindices[zz++] = yy+2; 
  tindices[zz++] = yy;   tindices[zz++] = yy+2;   tindices[zz++] = yy+3;

  //tex
  cubedata[0].tex[0] = cubedata[4].tex[0] = cubedata[21].tex[0] = -1; cubedata[0].tex[1] = cubedata[4].tex[1] = cubedata[21].tex[1] = -1; 
  cubedata[0].tex[2] = cubedata[4].tex[2] = cubedata[21].tex[2] = -1; 

  cubedata[1].tex[0] = cubedata[5].tex[0] =  cubedata[17].tex[0] = 1; cubedata[1].tex[1] = cubedata[5].tex[1] = cubedata[17].tex[1] = -1;
  cubedata[1].tex[2] = cubedata[5].tex[2] = cubedata[17].tex[2] = -1; 

  cubedata[2].tex[0] =  cubedata[8].tex[0] =  cubedata[18].tex[0] =  1; cubedata[2].tex[1] =  cubedata[8].tex[1] =  cubedata[18].tex[1] =   1;
  cubedata[2].tex[2] = cubedata[8].tex[2] = cubedata[18].tex[2] = -1; 

  cubedata[3].tex[0] = cubedata[9].tex[0] = cubedata[22].tex[0] = -1; cubedata[3].tex[1] =  cubedata[9].tex[1] =  cubedata[22].tex[1] =  1;
  cubedata[3].tex[2] = cubedata[9].tex[2] = cubedata[22].tex[2] = -1; 

  cubedata[12].tex[0] = cubedata[7].tex[0] = cubedata[20].tex[0] = -1; cubedata[12].tex[1] = cubedata[7].tex[1] = cubedata[20].tex[1] = -1;
  cubedata[12].tex[2] =  cubedata[7].tex[2] =  cubedata[20].tex[2] =  1; 

  cubedata[13].tex[0] =  cubedata[6].tex[0] = cubedata[16].tex[0] =  1; cubedata[13].tex[1] = cubedata[6].tex[1] = cubedata[16].tex[1] = -1;
  cubedata[13].tex[2] = cubedata[6].tex[2] = cubedata[16].tex[2] =  1; 

  cubedata[14].tex[0] = cubedata[11].tex[0] = cubedata[19].tex[0] =  1; cubedata[14].tex[1] = cubedata[11].tex[1] = cubedata[19].tex[1] =  1;
  cubedata[14].tex[2] =  cubedata[11].tex[2] =  cubedata[19].tex[2] =  1; 

  cubedata[15].tex[0] = cubedata[10].tex[0] = cubedata[23].tex[0] = -1; cubedata[15].tex[1] = cubedata[10].tex[1] = cubedata[23].tex[1] =  1;
  cubedata[15].tex[2] = cubedata[10].tex[2] = cubedata[23].tex[2] =  1; 

}

void SQVBO() 
{
	SQvalues();
	glGenBuffers(1, &vboHandle[1]);   // create an interleaved VBO object
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);   // bind the first handle 

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexWithTex)*4, SQ, GL_STATIC_DRAW); // allocate space and copy the position data over
	glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 

	glGenBuffers(1, &indexVBO[1]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*6, SQindices, GL_STATIC_DRAW);  // load the index data 

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

} 



void drawSQ(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,  GLuint c0, GLuint c1,
		   GLuint c2, GLuint m1, GLuint m2, GLuint m3, GLuint m4)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]);
	glEnableVertexAttribArray(c1);
	glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+0);  // position 
	glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+16); // normal
	glVertexAttribPointer(c1,3,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+48); // texture

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 
  	
  	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (char*)NULL+0); 

}

void InitCube_VBO() 
{

  cubeValue();

  glGenBuffers(1, &vboHandle[0]);   // create an interleaved VBO object
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);   // bind the first handle 

  glBufferData(GL_ARRAY_BUFFER, sizeof(VertexWithTex)*24, cubedata, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 


  glGenBuffers(1, &indexVBO[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*36, tindices, GL_STATIC_DRAW);  // load the index data 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

} 


void draw_cube(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,  GLuint c0,GLuint c1,
	       GLuint c2, GLuint m1, GLuint m2, GLuint m3, GLuint m4)
{

  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]);

	glEnableVertexAttribArray(c1);
	glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+0);  // position 
	glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+16); // normal
	glVertexAttribPointer(c1,3,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+48); // texture
  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 
  	
  	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (char*)NULL+0); 
}

VertexWithTex *PlyVertex;
GLuint *PlyOrder;

void PLYvalue(){

	PlyVertex = new VertexWithTex[vertexcount];
	PlyOrder = new GLuint[facecount*3];
	for (int i = 0; i < vertexcount; i++)
	{
		PlyVertex[i].location[0] = vertices[i] ->x;
		PlyVertex[i].location[1] = vertices[i] ->y;
		PlyVertex[i].location[2] = vertices[i] ->z;
		PlyVertex[i].location[3] = 1;

		PlyVertex[i].normal[0] = vertices[i] ->nx;
		PlyVertex[i].normal[1] = vertices[i] ->ny;
		PlyVertex[i].normal[2] = vertices[i] ->nz;
		PlyVertex[i].normal[3] = 0;

		// cout << vertices[i] ->nx << ", " << vertices[i] ->ny << ", " << vertices[i] ->nz << endl;
		// if((PlyVertex[i].normal[0]!=0) ||(PlyVertex[i].normal[1]!=0) ||(PlyVertex[i].normal[2]!=0)  )
		// 	cout << "WO" << endl;


		// PlyVertex[i].normal[0] = 1;
		// PlyVertex[i].normal[1] = 0;
		// PlyVertex[i].normal[2] = 0;
		// PlyVertex[i].normal[3] = 0;

	}
	for (int j = 0; j < facecount; j++)
	{
		PlyOrder[j*3] = faces[j]->vertices[0];
		PlyOrder[j*3+1] = faces[j]->vertices[1];
		PlyOrder[j*3+2] = faces[j]->vertices[2];
	}
}

VertexWithTex *PlyVertex2;
GLuint *PlyOrder2;

void PLYvalue2(){

	PlyVertex2 = new VertexWithTex[vertexcount2];
	PlyOrder2 = new GLuint[facecount2*3];
	for (int i = 0; i < vertexcount2; i++)
	{
		PlyVertex2[i].location[0] = vertices2[i] ->x;
		PlyVertex2[i].location[1] = vertices2[i] ->y;
		PlyVertex2[i].location[2] = vertices2[i] ->z;
		PlyVertex2[i].location[3] = 1;

		PlyVertex2[i].normal[0] = vertices2[i] ->nx;
		PlyVertex2[i].normal[1] = vertices2[i] ->ny;
		PlyVertex2[i].normal[2] = vertices2[i] ->nz;
		PlyVertex2[i].normal[3] = 0;

		// cout << vertices[i] ->nx << ", " << vertices[i] ->ny << ", " << vertices[i] ->nz << endl;
		// if((PlyVertex[i].normal[0]!=0) ||(PlyVertex[i].normal[1]!=0) ||(PlyVertex[i].normal[2]!=0)  )
		// 	cout << "WO" << endl;


		// PlyVertex[i].normal[0] = 1;
		// PlyVertex[i].normal[1] = 0;
		// PlyVertex[i].normal[2] = 0;
		// PlyVertex[i].normal[3] = 0;

	}
	for (int j = 0; j < facecount2; j++)
	{
		PlyOrder2[j*3] = faces2[j]->vertices[0];
		PlyOrder2[j*3+1] = faces2[j]->vertices[1];
		PlyOrder2[j*3+2] = faces2[j]->vertices[2];
	}
}

void PLYVBO(){
	PLYvalue();
	glGenBuffers(1, &vboHandle[1]);   // create an interleaved VBO object
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);   // bind the first handle 

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexWithTex)*vertexcount, PlyVertex, GL_STATIC_DRAW); // allocate space and copy the position data over
	glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 


	glGenBuffers(1, &indexVBO[1]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*facecount*3, PlyOrder, GL_STATIC_DRAW);  // load the index data 

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 
}
void PLYVBO2(){
	PLYvalue2();
	glGenBuffers(1, &vboHandle[2]);   // create an interleaved VBO object
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[2]);   // bind the first handle 

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexWithTex)*vertexcount2, PlyVertex2, GL_STATIC_DRAW); // allocate space and copy the position data over
	glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 


	glGenBuffers(1, &indexVBO[2]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[2]); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*facecount2*3, PlyOrder2, GL_STATIC_DRAW);  // load the index data 

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 
}

void drawPLY(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,  GLuint c0, GLuint c1,
		   GLuint c2, GLuint m1, GLuint m2, GLuint m3, GLuint m4)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]);
	glEnableVertexAttribArray(c1);
	glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+0);  // position 
	glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+16); // normal
	glVertexAttribPointer(c1,3,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+48); // texture

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 
  	
  	glDrawElements(GL_TRIANGLES, facecount*3, GL_UNSIGNED_INT, (char*)NULL+0); 

}

void drawPLY2(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,  GLuint c0, GLuint c1,
		   GLuint c2, GLuint m1, GLuint m2, GLuint m3, GLuint m4)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboHandle[2]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[2]);
	glEnableVertexAttribArray(c1);
	glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+0);  // position 
	glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+16); // normal
	glVertexAttribPointer(c1,3,GL_FLOAT, GL_FALSE, sizeof(VertexWithTex),(char*) NULL+48); // texture

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 
  	
  	glDrawElements(GL_TRIANGLES, facecount2*3, GL_UNSIGNED_INT, (char*)NULL+0); 

}

void display() 
{ 

////
   mat_shine[0] = 10;
  glClearColor(0,0,0,1); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  if (WIRE_FRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); 
  
  glUseProgram(programObject);

  GLuint c0 = glGetAttribLocation(programObject, "position");
  GLuint c1 = glGetAttribLocation(programObject, "tex");
  GLuint c2 = glGetAttribLocation(programObject, "normal");

  GLuint m1 = glGetUniformLocation(programObject, "local2clip");
  GLuint m2 = glGetUniformLocation(programObject, "local2eye");
  GLuint m3 = glGetUniformLocation(programObject, "normal_matrix");
  GLuint m4 = glGetUniformLocation(programObject, "world2eye");  
  

  GLuint Ia = glGetUniformLocation(programObject, "light_ambient");
  GLuint Id = glGetUniformLocation(programObject, "light_diffuse");
  GLuint Is = glGetUniformLocation(programObject, "light_specular");
  GLuint Lpos = glGetUniformLocation(programObject, "light_pos");

  GLuint Ka = glGetUniformLocation(programObject, "mat_ambient");
  GLuint Kd = glGetUniformLocation(programObject, "mat_diffuse");
  GLuint Ks = glGetUniformLocation(programObject, "mat_specular");
  GLuint Shine = glGetUniformLocation(programObject, "mat_shine"); 

  GLuint tt = glGetUniformLocation(programObject, "texture");  
  texture =0;
  glUniform1i(tt, texture);

  glUniform4f(Ia, light_ambient[0], light_ambient[1], light_ambient[2], light_ambient[3]);
  glUniform4f(Id, light_diffuse[0], light_diffuse[1], light_diffuse[2], light_diffuse[3]);
  glUniform4f(Is, light_specular[0], light_specular[1], light_specular[2], light_specular[3]);
  glUniform4f(Lpos, light_pos[0], light_pos[1], light_pos[2], light_pos[3]);

  glUniform4f(Ka, mat_ambient[0], mat_ambient[1], mat_ambient[2], mat_ambient[3]);
  glUniform4f(Kd, mat_diffuse[0], mat_diffuse[1], mat_diffuse[2], mat_diffuse[3]);
  glUniform4f(Ks, mat_specular[0], mat_specular[1], mat_specular[2], mat_specular[3]);
  glUniform1f(Shine, mat_shine[0]); 
  
  glEnableVertexAttribArray(c0);
  glEnableVertexAttribArray(c2);

  glm::mat4 view = glm::lookAt(glm::vec3(-10,10,0), 
                               glm::vec3(0.0, 0.0, 0.0), 
                               glm::vec3(0.0, 0.0, 1.0));
  
  glm::mat4 projection = glm::perspective(glm::radians(60.0f),1.0f,.01f,100.0f); 

  glm::mat4 model = glm::mat4(1.0f); 
  model = glm::rotate(model, glm::radians(z_angle), glm::vec3(0.0f, 0.0f, 1.0f)); 
  model = glm::rotate(model, glm::radians(x_angle), glm::vec3(1.0f, 0.0f, 0.0f)); 
  model = glm::scale(model, glm::vec3(scale_size, scale_size, scale_size));



  glm::mat4 mvp;
  glm::mat4 mv;
  glm::mat4 cubeM = model, plyM = model;
  int tex_loc;
  int tex_loc2;


  //BOX
  texture=2; //mirror surface

  glUniform1i(tt, texture);
   tex_loc2 = glGetUniformLocation(programObject, "CUBE2");
  glUniform1i(tex_loc2,1);
  glUniform4f(Kd, 1, 1, 1, 1);  
  

  plyM = glm::translate(plyM,glm::vec3(0.0, 0.0, -10));
  plyM = glm::scale(plyM, glm::vec3(15, 15, 12));
  mvp = projection*view*plyM;
  mv = view*plyM;
   draw_cube(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4); 

  //PLY
  texture=-1; //color surface

  glUniform1i(tt, texture);
   tex_loc = glGetUniformLocation(programObject, "CUBE");
  glUniform1i(tex_loc,0);
  glUniform4f(Kd, 0, 0, 1, 1);  
  plyM = model;
  plyM = glm::translate(plyM,glm::vec3(-2.0, -2.0,-5.0));
  plyM = glm::rotate(plyM, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
  plyM = glm::rotate(plyM, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
  plyM = glm::scale(plyM, glm::vec3(20.0, 20.0, 20.0));
  mvp = projection*view*plyM;
  mv = view*plyM;
   drawPLY(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  
  plyM = glm::translate(plyM,glm::vec3(0, 0.0, 0.2));
  mvp = projection*view*plyM;
  mv = view*plyM;
  glUniform4f(Kd, 1, 0, 0, 1);  
   drawPLY2(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  
  plyM = glm::translate(plyM,glm::vec3(0, 0.0, -0.2));




   texture=0; // mirror surface


  glUniform1i(tt, texture);
   tex_loc = glGetUniformLocation(programObject, "CUBE");
  glUniform1i(tex_loc,0);
  glUniform4f(Kd, 0, 1, 1, 1);  
  plyM = glm::translate(plyM,glm::vec3(0.3, 0.0, 0.0));
  mvp = projection*view*plyM;
  mv = view*plyM;
   drawPLY(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  
  plyM = glm::translate(plyM,glm::vec3(0, 0.0, 0.2));
  mvp = projection*view*plyM;
  mv = view*plyM;
   drawPLY2(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  


  //draw_cube(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  

///
  // Skybox
  texture = 1; // cube 

  glUniform1i(tt, texture);
   tex_loc = glGetUniformLocation(programObject, "CUBE");
  glUniform1i(tex_loc,0);


  glUniform4f(Kd, 1, 1, 1, 1);  

  cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 40.0));
  mvp = projection*view*cubeM;
  mv = view*cubeM;
  draw_cube(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);  

 


  // //z

  // // glUniform4f(Kd, 0, 1, 1, 1);  
  // cubeM = glm::translate(cubeM,glm::vec3(0.0, 0.0, -20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);   

  // // glUniform4f(Kd, 1, 0, 1, 1);  
  // cubeM = glm::translate(model,glm::vec3(0.0, 0.0, 20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4); 

  // //x
  // // glUniform4f(Kd, 1, 1, 0, 1);  
  // cubeM = glm::rotate(model,glm::radians(90.0f),glm::vec3(1, 0, 0));
  // cubeM = glm::rotate(cubeM,glm::radians(90.0f),glm::vec3(0, 1, 0));
  // cubeM = glm::translate(cubeM,glm::vec3(0.0, 0.0, -20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);   

  // // glUniform4f(Kd, 1, 0, 0, 1);  
  // cubeM = glm::rotate(model,glm::radians(90.0f),glm::vec3(1, 0, 0));
  // cubeM = glm::rotate(cubeM,glm::radians(90.0f),glm::vec3(0, 1, 0));
  // cubeM = glm::translate(cubeM,glm::vec3(0.0, 0.0, 20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4); 

  // //y
  // // glUniform4f(Kd, 0, 1, 0, 1);  
  // cubeM = glm::rotate(model,glm::radians(90.0f),glm::vec3(1, 0, 0));
  // cubeM = glm::translate(cubeM,glm::vec3(0.0, 0.0, 20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4);   

  // // glUniform4f(Kd, 0, 0, 1, 1);  
  // cubeM = glm::rotate(model,glm::radians(90.0f),glm::vec3(1, 0, 0));
  // cubeM = glm::translate(cubeM,glm::vec3(0.0, 0.0, -20));
  // cubeM = glm::scale(cubeM, glm::vec3(40.0, 40.0, 1.0));
  // mvp = projection*view*cubeM;
  // mv = view*cubeM;
  // drawSQ(mvp, mv, &view[0][0], c0, c1, c2, m1, m2, m3, m4); 


  glDisableClientState(GL_VERTEX_ARRAY); 

  glutSwapBuffers(); 

} 

///////////////////////////////////////////////////////////////

void mymotion(int x, int y)
{
	if (xform_mode==XFORM_ROTATE) {
	  z_angle += (x - press_x)/5.0; 
	  if (z_angle > 180) z_angle -= 360; 
	  else if (z_angle <-180) z_angle += 360; 
	  press_x = x; 
		   
	  x_angle -= (y - press_y)/5.0; 
	  if (x_angle > 180) x_angle -= 360; 
	  else if (x_angle <-180) x_angle += 360; 
	  press_y = y; 
	}
		else if (xform_mode == XFORM_SCALE){
	  float old_size = scale_size;
	  scale_size *= (1+ (y - press_y)/60.0); 
	  if (scale_size <0) scale_size = old_size; 
	  press_y = y; 
	}
	glutPostRedisplay(); 
}
void mymouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) {
	press_x = x; press_y = y; 
	if (button == GLUT_LEFT_BUTTON)
	  xform_mode = XFORM_ROTATE; 
		 else if (button == GLUT_RIGHT_BUTTON) 
	  xform_mode = XFORM_SCALE; 
  }
  else if (state == GLUT_UP) {
		  xform_mode = XFORM_NONE; 
  }
}


void mykey(unsigned char key, int x, int y)
{
	float d_angle = 10; 
	if (key == 'q') exit(1); 
	if (key == 'R') 
	  modelM = glm::rotate(modelM, glm::radians(d_angle), glm::vec3(0.0f, 0.0f, 1.0f)); 
	if (key == 'r') 
	  modelM = glm::translate(modelM, glm::vec3(0.1f, 0.0f, 0.0f)); 
	if (key == 'l') 
	  modelM = glm::translate(modelM, glm::vec3(-0.1f, 0.0f, 0.0f)); 
	if (key == 'f') 
	  modelM = glm::translate(modelM, glm::vec3(0.0f, 0.1f, 0.0f)); 
	if (key == 'b') 
	  modelM = glm::translate(modelM, glm::vec3(0.0f, -0.1f, 0.0f)); 
	if (key == 'c') {
	  modelM =  glm::mat4(1.0f);
	  angle1 = angle2 = angle3 = angle4 = 0; 
	}
	if (key == 's') {
	  WIRE_FRAME = !WIRE_FRAME;
	}

	if (key == '1') {
		  angle1 += 5; 
		  // printf(" hello!\n"); 
		}
		if (key == '2') 
		  angle2 += 5;
		if (key == '-') {
		  mat_shine[0] += 1;
	  if (mat_shine[0] < 1) mat_shine[0] = 1; 
	}
		if (key == '=') 
		  mat_shine[0] -= 1;    
		if (key == '3') 
	  light_pos[0] += 0.5; 
		if (key == '4') 
	  light_pos[0] -= 0.5; 
		if (key == '5') 
	  light_pos[1] += 0.5; 
		if (key == '6') 
	  light_pos[1] -= 0.5; 
		if (key == '7') 
	  light_pos[2] += 0.5; 
		if (key == '8') 
	  light_pos[2] -= 0.5; 

	
	glutPostRedisplay(); 
}

int main(int argc, char** argv) 
{ 
  PlyFile* input;
  FILE *realFile1 = fopen("bunny2.ply", "r");
  FILE *realFile2 = fopen("dragon2.ply", "r");

  // get the ply structure and open the file
  input = read_ply(realFile1);

  // read in the data
  store_ply(input, 
	    &vertices, &faces, 
	    &vertexcount, &facecount,
	    &vertexnormals, &facenormals);
  cout << "PLY Vertex: "<< vertexcount << endl;
  cout << "PLY Faces: "<< facecount << endl;

  // get the ply structure and open the file
  input = read_ply(realFile2);

  // read in the data
  store_ply(input, 
	    &vertices2, &faces2, 
	    &vertexcount2, &facecount2,
	    &vertexnormals2, &facenormals2);
  cout << "PLY2 Vertex: "<< vertexcount2 << endl;
  cout << "PLY2 Faces: "<< facecount2 << endl;

  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH); 
  glutInitWindowSize(800,800); 

  glutCreateWindow("lab5"); 
  glutDisplayFunc(display); 
  glutMouseFunc(mymouse); 
  glutKeyboardFunc(mykey);
  glutMotionFunc(mymotion);

  glewInit(); 

  SQVBO();
  PLYVBO();
  PLYVBO2();
  InitCube_VBO();
  programObject = SetupGLSL("lab5");  
  // INTtexture();
  MapVBO();
  MapVBO2();

  glutMainLoop(); 

} 
