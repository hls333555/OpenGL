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

#define NUM_POINT_LIGHTS 5

struct Material
{
	sampler2D diffuseTex1;
	sampler2D diffuseTex2;

	bool bHasSpecular;
	sampler2D specularTex1;
	sampler2D specularTex2;

	float shininess;
};

struct PointLight
{
	vec3 position;

	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;

	// Attenuation = 1.f / (q * d * d + l * d + c)
	float q;
	float l;
	float c;
};

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal);

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 color;

uniform Material u_Material;
uniform PointLight u_PointLights[NUM_POINT_LIGHTS];
uniform vec3 u_ViewPos;

void main()
{
	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 normalizedNorm = normalize(v_Normal);

	vec3 finalColor = vec3(0.f);
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		vec3 pointColor = CalculatePointLight(u_PointLights[i], v_FragPos, viewDir, normalizedNorm);
		finalColor += pointColor;
	}

	color = vec4(finalColor, 1.f);
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	vec3 ambientColor = vec3(texture(u_Material.diffuseTex1, v_TexCoord)) * light.ambientIntensity;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex1, v_TexCoord)) * diff * light.diffuseIntensity * attenuation;

	vec3 specularColor = vec3(0.f);
	if (u_Material.bHasSpecular)
	{
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
		specularColor = vec3(texture(u_Material.specularTex1, v_TexCoord)) * spec * light.specularIntensity * attenuation;
	}

	return ambientColor + diffuseColor + specularColor;
}
