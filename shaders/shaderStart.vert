#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 lightPosEye; 
out vec4 fragPosEye;
out vec2 fragTexCoords;

uniform vec4 lightPos;

uniform mat4 lightSpaceTrMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform	mat3 normalMatrix;

void main()
{
	//compute eye space coordinates
	fragPosEye = view * model * vec4(vPosition, 1.0f);
	fNormal = normalize(normalMatrix * vNormal);
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	lightPosEye = view * model * lightPos;
	fragTexCoords = vTexCoords;
}