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
	vs_out.v_Normal = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);
	vs_out.v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

// MRT
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 brightColor;

struct Material
{
	sampler2D diffuseTex;
	sampler2D specularTex;
	float shininess;
};

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

in VS_OUT
{
	vec3 v_FragPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} fs_in;

uniform Material u_Material;
uniform PointLight u_PointLights[3];
uniform vec3 u_ViewPos;

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal);

void main()
{
	vec3 viewDir = normalize(u_ViewPos - fs_in.v_FragPos);

	vec3 pointColor = vec3(0.f);
	for (int i = 0; i < 3; ++i)
	{
		pointColor += CalculatePointLight(u_PointLights[i], fs_in.v_FragPos, viewDir, fs_in.v_Normal);
	}

	// Calculate the brightness by transforming it to greyscale
	float brightness = dot(pointColor, vec3(0.2126f, 0.7152f, 0.0722f));
	if (brightness > 1.f)
	{
		brightColor = vec4(pointColor, 1.f);
	}
	else
	{
		brightColor = vec4(0.f, 0.f, 0.f, 1.f);
	}
	fragColor = vec4(pointColor, 1.f);
	// Comment this line to enable bloom for cubes!
	brightColor = vec4(0.f, 0.f, 0.f, 1.f);
}

vec3 CalculatePointLight(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal)
{
	// PointLight attenuation
	float dist = length(light.position - fragPos);
	float attenuation = 1.f / (light.q * dist * dist + light.l * dist + light.c);

	vec3 ambientColor = texture(u_Material.diffuseTex, fs_in.v_TexCoord).rgb * light.ambientIntensity;

	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = texture(u_Material.diffuseTex, fs_in.v_TexCoord).rgb * light.color * diff * light.diffuseIntensity * attenuation;

	// Phong model
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	// Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.f), u_Material.shininess);
	vec3 specularColor = texture(u_Material.specularTex, fs_in.v_TexCoord).rgb * light.color * spec * light.specularIntensity * attenuation;

	return ambientColor + diffuseColor + specularColor;
}
