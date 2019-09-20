#shader vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 0.f, 1.f);
}

#shader fragment
#version 330 core

#define NUM_POINT_LIGHTS 5

struct PointLight
{
	vec3 position;
	vec3 color;

	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;

	// Attenuation = 1.f / (q * d * d + l * d + c)
	float q;
	float l;
	float c;
};

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 diffuse, float specular, vec3 viewDir);

in vec2 v_TexCoord;

out vec4 color;

uniform PointLight u_PointLights[NUM_POINT_LIGHTS];
uniform vec3 u_ViewPos;

uniform sampler2D u_GPosition;
uniform sampler2D u_GNormal;
uniform sampler2D u_GColorSpec;

void main()
{
	// Retrieve data from GBuffer
	vec3 fragPos = texture(u_GPosition, v_TexCoord).rgb;
	vec3 normal = texture(u_GNormal, v_TexCoord).rgb;
	vec3 diffuse = texture(u_GColorSpec, v_TexCoord).rgb;
	float specular = texture(u_GPosition, v_TexCoord).a;

	vec3 viewDir = normalize(u_ViewPos - fragPos);

	vec3 finalColor = vec3(0.f);
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		vec3 pointColor = CalculatePointLight(u_PointLights[i], fragPos, normal, diffuse, specular, viewDir);
		finalColor += pointColor;
	}

	color = vec4(finalColor, 1.f);
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 diffuse, float specular, vec3 viewDir)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	vec3 ambientColor = diffuse * light.ambientIntensity;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = diffuse * diff * light.color * light.diffuseIntensity * attenuation;

	vec3 specularColor = vec3(0.f);
	if (specular != 0.f)
	{
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float shininess = 64.f;
		float spec = pow(max(dot(normal, halfwayDir), 0.f), shininess);
		specularColor = specular * spec * light.color * light.specularIntensity * attenuation;
	}

	return ambientColor + diffuseColor + specularColor;
}
