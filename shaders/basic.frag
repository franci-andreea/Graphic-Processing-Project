#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPosEye;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix; //used to transform the normal for diffuse lighting

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components used for lights
//ambient light
vec3 ambient;
float ambientStrength = 0.2f;
//diffuse light
vec3 diffuse;
//specular light
vec3 specular;
float specularStrength = 0.5f;

float shininess = 32.0f;

//fog uniforms
uniform float fogDensity;
uniform float transparency;

//use this function to compute how dense the fog should be according to the viewer's position
float computeFog()
{
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void computeDirLight()
{
    //compute ambient light
    ambient = ambientStrength * lightColor;

    //normalize light direction
    vec3 lightDirN = normalize(lightDir);

    //compute eye space coordinates for normals
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDirN = normalize(-fPosEye.xyz);

    //compute the light's reflection
    vec3 reflectDir = normalize(reflect(-lightDirN, normalEye));
    
    //compute specular light
    float specCoeff = pow(max(dot(viewDirN, reflectDir), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

void main() 
{
    computeDirLight();

    // //compute final vertex color
    // vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

    //vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    vec3 color = min((ambient + diffuse)  + specular , 1.0f);

    //compute fog
    if(fogDensity != 0.0f)
    {
        float fogFactor = computeFog();
	    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        vec4 colorFromTexture = texture(diffuseTexture, fTexCoords);
		if(colorFromTexture.a < 0.1)
            discard;
        fColor = colorFromTexture;
		fColor = vec4(color, 1.0f);
		fColor = vec4(vec3(fogColor * (1 - fogFactor) + vec4(color, colorFromTexture.a) * fogFactor), 1.0f);
    }
    else
    {
		fColor = vec4(color, transparency);
	}
}
