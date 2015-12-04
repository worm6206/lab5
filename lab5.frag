varying vec3 N;
varying vec3 L;
varying vec3 R;
varying vec3 V;
varying vec4 Vpos;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;
uniform vec4 light_pos;

uniform vec4 mat_ambient;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform float mat_shine; 

void main() { 

	//ambient
	vec4 ambient = light_ambient * mat_ambient;
	//diffuse
	float NdotL = dot(N,L);
	if (NdotL < 0.0) NdotL = 0.0;
	vec4 diffuse = light_diffuse * mat_diffuse * NdotL;
	//specular
	float RdotV = dot(R,V);
	if(RdotV < 0.0) RdotV = 0.0;
	if(NdotL == 0.0) RdotV = 0.0;
	vec4 specular = pow(RdotV,mat_shine) * light_specular * mat_specular;

    //gl_FragColor = pcolor; 
    gl_FragColor = diffuse + ambient + specular;
	
 } 
