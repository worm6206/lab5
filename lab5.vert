uniform int texture;

attribute vec4 position; 
attribute vec4 normal; 
attribute vec3 tex;

varying vec3 texC;

varying vec3 N;
varying vec3 L;
varying vec3 R;
varying vec3 V;
varying vec4 Vpos;

varying vec3 reflectV;
varying vec3 staticV;

uniform mat4 local2clip;
uniform mat4 local2eye;
uniform mat4 normal_matrix;
uniform mat4 world2eye; 

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;
uniform vec4 light_pos;

uniform vec4 mat_ambient;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform float mat_shine; 



void main(){


       gl_Position = local2clip * position;
       
       N =     normalize(vec3(normal_matrix * -normal)); 
       vec4 Lpos =  world2eye * light_pos; 
       Vpos =  local2eye * position; 
       L = normalize(vec3(Lpos - Vpos)); 


       R = normalize(reflect(-L, N)); 
       V = normalize(vec3(-Lpos)); 
       gl_Position = local2clip * position;
       texC = tex;

       reflectV = reflect(vec3(position - Vpos), N);
       staticV = reflect(V, N);
}
