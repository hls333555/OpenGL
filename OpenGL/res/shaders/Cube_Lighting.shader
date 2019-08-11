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

#define NUM_POINT_LIGHTS 3

struct Material
{
	sampler2D diffuseTex;
	sampler2D specularTex;
	float shininess;
};

struct DirectionalLight
{
	vec3 direction;

	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;
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

struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;

	// Should be value of cos
	float innerAngle;
	float outerAngle;

	// Attenuation = 1.f / (q * d * d + l * d + c)
	float q;
	float l;
	float c;
};

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, vec3 normal);
vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal);
vec3 CalculateSpotLight(SpotLight light, vec3 fragPos, vec3 viewDir, vec3 normal);

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 color;

uniform Material u_Material;
uniform DirectionalLight u_DirLight;
uniform PointLight u_PointLights[NUM_POINT_LIGHTS];
uniform SpotLight u_SpotLight;
uniform vec3 u_ViewPos;

uniform bool u_EnableDirLight;
uniform bool u_EnablePointLights;
uniform bool u_EnableSpotLight;

void main()
{
	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 normalizedNorm = normalize(v_Normal);

	vec3 finalColor = vec3(0.f);
	if (u_EnableDirLight)
	{
		vec3 directionalColor = CalculateDirectionalLight(u_DirLight, viewDir, normalizedNorm);
		finalColor += directionalColor;
	}
	if (u_EnablePointLights)
	{
		for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			vec3 pointColor = CalculatePointLight(u_PointLights[i], v_FragPos, viewDir, normalizedNorm);
			finalColor += pointColor;
		}
	}
	if (u_EnableSpotLight)
	{
		vec3 spotLight = CalculateSpotLight(u_SpotLight, v_FragPos, viewDir, normalizedNorm);
		finalColor += spotLight;
	}
	color = vec4(finalColor, 1.f);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, vec3 normal)
{
	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * light.ambientIntensity;

	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * diff * light.diffuseIntensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, v_TexCoord)) * spec * light.specularIntensity;

	return ambientColor + diffuseColor + specularColor;
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * light.ambientIntensity;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * diff * light.diffuseIntensity * attenuation;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, v_TexCoord)) * spec * light.specularIntensity * attenuation;

	return ambientColor + diffuseColor + specularColor;
}

vec3 CalculateSpotLight(SpotLight light, vec3 fragPos, vec3 viewDir, vec3 normal)
{
	// SpotLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	// SpotLight intensity
	vec3 lightDir = normalize(light.position - fragPos);
	float theta = dot(normalize(-light.direction), lightDir);
	float epsilon = light.innerAngle - light.outerAngle;
	float intensity = clamp((theta - light.outerAngle) / epsilon, 0.f, 1.f);

	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * light.ambientIntensity * intensity;

	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * diff * light.diffuseIntensity * attenuation * intensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, v_TexCoord)) * spec * light.specularIntensity * attenuation * intensity;

	return ambientColor + diffuseColor + specularColor;
}
