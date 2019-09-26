#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out VS_OUT
{
	vec3 v_FragPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} vs_out;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	vs_out.v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	vs_out.v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	vs_out.v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

#define MAX_POINT_LIGHTS 4

in VS_OUT
{
	vec3 v_FragPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} fs_in;

out vec4 color;

struct PointLight
{
	vec3 position;

	//vec3 ambientIntensity;
	//vec3 diffuseIntensity;
	//vec3 specularIntensity;
};

uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform vec3 u_ViewPos;

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal);

void main()
{
	vec3 viewDir = normalize(u_ViewPos - fs_in.v_FragPos);
	vec3 normalizedNorm = normalize(fs_in.v_Normal);

	vec3 pointColor = vec3(0.f);
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		pointColor += CalculatePointLight(u_PointLights[i], fs_in.v_FragPos, viewDir, normalizedNorm);
	}

	color = vec4(pointColor, 1.f);
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (dist * dist);

	vec3 ambientColor = vec3(1.f) * vec3(0.1f);

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(1.f) * diff * vec3(1.f) * attenuation;

	// Phong model
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), 64);
	// Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.f), 64);
	vec3 specularColor = vec3(1.f) * spec * vec3(1.f) * attenuation;

	return ambientColor + diffuseColor + specularColor;
}
