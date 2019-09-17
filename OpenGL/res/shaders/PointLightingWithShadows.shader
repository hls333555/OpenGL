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
uniform mat4 u_LightSpaceViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	vs_out.v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	//vs_out.v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	vs_out.v_Normal = a_Normal;
	vs_out.v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuseTex;
	sampler2D specularTex;
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

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float shadow);
float ShadowCalculation(vec3 fragPos);

in VS_OUT
{
	vec3 v_FragPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} fs_in;

out vec4 color;

uniform Material u_Material;
uniform PointLight u_PointLight;
uniform vec3 u_ViewPos;
uniform samplerCube u_ShadowCubemap;
uniform float u_FarPlane;

void main()
{
	vec3 viewDir = normalize(u_ViewPos - fs_in.v_FragPos);
	vec3 normalizedNorm = normalize(fs_in.v_Normal);

	float shadow = ShadowCalculation(fs_in.v_FragPos);

	vec3 pointColor = CalculatePointLight(u_PointLight, fs_in.v_FragPos, viewDir, normalizedNorm, shadow);

	color = vec4(pointColor, 1.f);
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float shadow)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, fs_in.v_TexCoord)) * light.ambientIntensity;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, fs_in.v_TexCoord)) * diff * light.diffuseIntensity * attenuation;

	// Phong model
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	// Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, fs_in.v_TexCoord)) * spec * light.specularIntensity * attenuation;

	return ambientColor + (1.f - shadow) * (diffuseColor + specularColor);
}

float ShadowCalculation(vec3 fragPos)
{
	// Get direction vector between fragment position and light position
	vec3 lightDir = fragPos - u_PointLight.position;
	// Use the direction vector to sample from the depth map
	float closestDepth = texture(u_ShadowCubemap, lightDir).r;
	// It is currently in linear range between [0,1], re-transform back to original value
	closestDepth *= u_FarPlane;
	// Get current linear depth as the length between the fragment position and light position
	float currentDepth = length(lightDir);
	// To solve shadow acne
	float bias = 0.1f;
	float shadow = 0.f;
	shadow = currentDepth - bias > closestDepth ? 1.f : 0.f;
	// To solve aliasing via PCF

	return shadow;
}
