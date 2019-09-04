#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out VS_OUT
{
	vec3 clipSpaceNormal;
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} vs_out;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.f);

	mat3 normalMatrix = mat3(transpose(inverse(u_View * u_Model)));
	// GS receives its position vectors as clip-space coordinates
	// so we should also transform the normal vectors to the same space
	vs_out.clipSpaceNormal = normalize(vec3(u_Projection * vec4(normalMatrix * a_Normal, 1.f)));

	vs_out.fragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	vs_out.normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	vs_out.texCoord = a_TexCoord;
}	

#shader geometry
#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 clipSpaceNormal;
	vec3 fragPos;
	vec3 normal;
	vec2 texCoord;
} gs_in[];

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

uniform float u_Time;

#define EXPLODE_MAG 1.0f

vec4 explode(vec4 vertexPosition, vec3 vertexNormal);

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = explode(gl_in[i].gl_Position, gs_in[i].clipSpaceNormal);
		v_FragPos = gs_in[i].fragPos;
		v_Normal = gs_in[i].normal;
		v_TexCoord = gs_in[i].texCoord;
		EmitVertex();
	}
	EndPrimitive();
}

vec4 explode(vec4 vertexPosition, vec3 vertexNormal)
{
	vec3 explodeOffset = vertexNormal * (sin(u_Time) + 1.f) / 2.f * EXPLODE_MAG;
	return vertexPosition + vec4(explodeOffset, 0.f);
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
