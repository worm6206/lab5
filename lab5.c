
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <stack> 
#include <math.h> 

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

using namespace std;

#include<glm/glm.hpp> 
#include<glm/gtx/transform.hpp>

GLuint programObject;
GLuint SetupGLSL(char*); 

typedef struct 
{
  float location[4]; 
  float normal[4]; 
  float color[4]; 
} Vertex; 

int nindices; 
Vertex *cyverts;   
Vertex *cubedata;
GLuint *cindices; 

GLubyte tindices[36];  

GLuint vboHandle[2];   
GLuint indexVBO[2]; 

GLubyte readImage[400][400][4]; 
GLubyte texImage[256][256][4]; 

float angle1=0, angle2=0;
float angle3=0, angle4=0;

glm::mat4 modelM = glm::mat4(1.0f); 

stack<glm::mat4> mat_stack;


GLfloat light_ambient[4] = {0.8,0.8,0.8,1};  //Ia 
GLfloat light_diffuse[4] = {0.8,0.8,0.8,1};  //Id
GLfloat light_specular[4] = {1,1,1,1};  //Is
GLfloat light_pos [4] = {4, 4, 1.8, 1};


GLfloat mat_ambient[4] = {0.1,0.1,0.1,1};  //Ka 
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

void InitCylinder_VBO(int nslices, int nstacks, float r, float g, float b) 
{

  int nvertices = nslices * nstacks; 
  cyverts = new Vertex[nvertices]; 

  float Dangle = 2*M_PI/(float)(nslices-1); 

  for (int j =0; j<nstacks; j++)
    for (int i=0; i<nslices; i++) {
      int idx = j*nslices + i; // mesh[j][i] 
      float angle = Dangle * i; 
      cyverts[idx].location[0] = cyverts[idx].normal[0] = cos(angle); 
      cyverts[idx].location[1] = cyverts[idx].normal[1] = sin(angle); 
      cyverts[idx].location[2] = j*1.0/(float)(nstacks-1); 
      cyverts[idx].normal[2] = 0.0; 
      cyverts[idx].location[3] = 1.0;  cyverts[idx].normal[3] = 0.0; 
    }

  nindices = (nstacks-1)*2*(nslices+1); 
  cindices = new GLuint[nindices]; 
  int n = 0; 
  for (int j =0; j<nstacks-1; j++)
    for (int i=0; i<=nslices; i++) {
      int mi = i % nslices;  
      int idx = j*nslices + mi; // mesh[j][mi] 
      int idx2 = (j+1) * nslices + mi; 
      cindices[n++] = idx; 
      cindices[n++] = idx2; 
    }

  glGenBuffers(1, &vboHandle[1]);   // create an interleaved VBO object
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);   // bind the first handle 

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*nvertices, cyverts, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 

  glGenBuffers(1, &indexVBO[1]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*nindices, cindices, GL_STATIC_DRAW);  // load the index data 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

}

void InitCube_VBO() 
{

  /*
  So this part is about defining cube's endpoints and their normals.
  I thought that one point can be included in 3 planes, which are facing different axis, thus should have
  differnt normal. So below is what it's about. The result is better than professor Shen's implementation,
  but not what I expected to be, didn't fix it due to time constrain.
  */
  //6planes * 4points form a plane = 24
  cubedata = new Vertex[24];

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


  glGenBuffers(1, &vboHandle[0]);   // create an interleaved VBO object
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);   // bind the first handle 

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*24, cubedata, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 
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

  glGenBuffers(1, &indexVBO[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*(zz), tindices, GL_STATIC_DRAW);  // load the index data 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 

} 


void draw_cube(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,  GLuint c0,
	       GLuint c2, GLuint m1, GLuint m2, GLuint m3, GLuint m4)
{

  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]);

  glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, 48,(char*) NULL+0);  // position 
  glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, 48,(char*) NULL+16); // normal 

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (char*)NULL+0); 
}

void draw_cylinder(glm::mat4 local2clip, glm::mat4 local2eye, float* world2eye,
		    GLuint c0, GLuint c2, GLuint m1,
		   GLuint m2, GLuint m3, GLuint m4)
{
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[1]);
  glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, 48,(char*) NULL+0);  // position 
  glVertexAttribPointer(c2,4,GL_FLOAT, GL_FALSE, 48,(char*) NULL+16); // normal

  glm::mat4 normal_matrix = glm::inverse(local2eye);
  normal_matrix = glm::transpose(normal_matrix);

  glUniformMatrix4fv(m1, 1, GL_FALSE, (float*) &local2clip[0][0]);   // pass the local2clip matrix
  glUniformMatrix4fv(m2, 1, GL_FALSE, (float*) &local2eye[0][0]);   // pass the local2eye matrix
  glUniformMatrix4fv(m3, 1, GL_FALSE, (float*) &normal_matrix[0][0]);   // pass the normal matrix
  glUniformMatrix4fv(m4, 1, GL_FALSE, (float*) world2eye);   // pass the w2e matrix 

  glDrawElements(GL_TRIANGLE_STRIP, nindices, GL_UNSIGNED_INT, (char*) NULL+0); 

}

void display() 
{ 
  glClearColor(0,0,0,1); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  if (WIRE_FRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); 
  
  glUseProgram(programObject);

  GLuint c0 = glGetAttribLocation(programObject, "position");
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

  glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 0), 
                               glm::vec3(0.0, -1.0, 0.0), 
                               glm::vec3(0.0, 0.0, 1.0));
  
  glm::mat4 projection = glm::perspective(glm::radians(60.0f),1.0f,.01f,100.0f); 

  glm::mat4 model = glm::mat4(1.0f); 
  model = glm::rotate(model, glm::radians(z_angle), glm::vec3(0.0f, 0.0f, 1.0f)); 
  model = glm::rotate(model, glm::radians(x_angle), glm::vec3(1.0f, 0.0f, 0.0f)); 
  model = glm::scale(model, glm::vec3(scale_size, scale_size, scale_size));


  glm::mat4 mvp;
  glm::mat4 mv;
  

// light and floor

  glm::mat4 lightM = model; 
  lightM = glm::translate(lightM, glm::vec3(light_pos[0], light_pos[1], light_pos[2])); 
  lightM = glm::scale(lightM, glm::vec3(0.5, 0.5, 0.5));

  mvp = projection*view*lightM;
  mv = view*lightM;

  glUniform4f(Kd, 0.5, 1, 1, 1); 

  draw_cube(mvp, mv, &view[0][0], c0, c2, m1, m2, m3, m4);


// objects for lighting demo

  mat_stack.push(modelM);

  modelM = glm::translate(modelM, glm::vec3(0.0f, 0.0f, 0.0f));
  
  mat_stack.push(modelM);  
  modelM = glm::scale(modelM, glm::vec3(25.0f, 25.0f, 25.0f));
  mvp = projection*view*model*modelM;
  mv = view*model*modelM;
  
  glUniform4f(Kd, 0, 0.2, 0.7, 1);   
  draw_cube(mvp, mv, &view[0][0], c0, c2, m1, m2, m3, m4);  
  
  modelM = mat_stack.top();  mat_stack.pop();

  glEnable(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256,0, GL_RGBA, GL_UNSIGNED_BYTE, texImage);
  glDisable(GL_TEXTURE_2D);

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

  glutInit(&argc, argv); 
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH); 
  glutInitWindowSize(800,800); 

  glutCreateWindow("lab5"); 
  glutDisplayFunc(display); 
  glutMouseFunc(mymouse); 
  glutKeyboardFunc(mykey);
  glutMotionFunc(mymotion);

  glewInit(); 

  InitCube_VBO();

  InitCylinder_VBO(10,10,1,1,0);

  programObject = SetupGLSL("lab5");  

  glutMainLoop(); 

} 
