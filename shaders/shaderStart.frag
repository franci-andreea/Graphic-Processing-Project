#version 410 core

in vec2 fragTexCoords;
in vec3 fNormal;
in vec4 lightPosEye;
in vec4 fragPosEye;

out vec4 fColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//lighting
uniform	vec3 lightColor;

vec3 ambient;
float ambientStrength = 0.2f;

vec3 diffuse;

vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

void main()
{
	vec3 cameraPosEye = vec3(0.0f); //in eye coordinates, the viewer is situated at the origin

	//compute distance to light
	//float dist = length(lightPosEye.xyz - fragPosEye.xyz);

	//compute attenuation
	//float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//transform normal
	//vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	//vec3 lightDirN = normalize(lightPosEye.xyz - fragPosEye.xyz);
	
	//compute view direction 
	//vec3 viewDirN = normalize( - fragPosEye.xyz);
		
	//compute ambient light
	//ambient = att * ambientStrength * lightColor;
	ambient = texture(diffuseTexture, fragTexCoords).xyz;
	
	//compute diffuse light
	//diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	//diffuse *= texture(diffuseTexture, fragTexCoords);
	
	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	//float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	//specular = att * specularStrength * specCoeff * lightColor;
	specular *= texture(specularTexture, fragTexCoords);

	//vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); //orange
	
	//ambient *=  baseColor;
	//diffuse *= baseColor;
	//specular *= baseColor;
	
	//vec3 color = min((ambient + diffuse) + specular, 1.0f); 

	fColor = vec4(ambient, 1.0f);
}