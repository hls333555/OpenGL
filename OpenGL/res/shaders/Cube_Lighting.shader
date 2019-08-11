#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;
}	

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuseTex;
	sampler2D specularTex;
	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;
};

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 color;

uniform Material u_material;
uniform Light u_light;
uniform vec3 u_ViewPos;

void main()
{
	vec3 ambientColor = vec3(texture(u_material.diffuseTex, v_TexCoord)) * u_light.ambientIntensity;

	vec3 lightDir = normalize(u_light.position - v_FragPos);
	vec3 normalizedNorm = normalize(v_Normal);
	float diff = max(dot(normalizedNorm, lightDir), 0.f);
	vec3 diffuseColor = diff * vec3(texture(u_material.diffuseTex, v_TexCoord)) * u_light.diffuseIntensity;

	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, normalizedNorm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_material.shininess);
	vec3 specularColor = spec * vec3(texture(u_material.specularTex, v_TexCoord)) * u_light.specularIntensity;

	color = vec4(ambientColor + diffuseColor + specularColor, 1.f);
}
