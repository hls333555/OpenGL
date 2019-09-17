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
	vec4 v_FragPosLightSpace;
} vs_out;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform mat4 u_LightSpaceViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	vs_out.v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	vs_out.v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	vs_out.v_TexCoord = a_TexCoord;
	vs_out.v_FragPosLightSpace = u_LightSpaceViewProjection * vec4(vs_out.v_FragPos, 1.f);
}

#shader fragment
#version 330 core

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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, vec3 normal, float shadow);
float ShadowCalculation(vec4 fragPosLightSpace);

in VS_OUT
{
	vec3 v_FragPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
	vec4 v_FragPosLightSpace;
} fs_in;

out vec4 color;

uniform Material u_Material;
uniform DirectionalLight u_DirLight;
uniform vec3 u_ViewPos;
uniform sampler2D u_ShadowMap;

void main()
{
	vec3 viewDir = normalize(u_ViewPos - fs_in.v_FragPos);
	vec3 normalizedNorm = normalize(fs_in.v_Normal);

	float shadow = ShadowCalculation(fs_in.v_FragPosLightSpace);

	vec3 directionalColor = CalculateDirectionalLight(u_DirLight, viewDir, normalizedNorm, shadow);

	color = vec4(directionalColor, 1.f);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, vec3 normal, float shadow)
{
	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, fs_in.v_TexCoord)) * light.ambientIntensity;

	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, fs_in.v_TexCoord)) * diff * light.diffuseIntensity;

	// Phong model
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	// Blinn-Phong model
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, fs_in.v_TexCoord)) * spec * light.specularIntensity;

	return ambientColor + (1.f - shadow) * (diffuseColor + specularColor);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// Perform perspective divide
	vec3 projCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	projCoord = projCoord * 0.5f + 0.5f;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoord.z;
	// To solve shadow acne
	//float bias = 0.005f;
	vec3 normalizedNorm = normalize(fs_in.v_Normal);
	float bias = max(0.05f * (1.f - dot(normalizedNorm, -u_DirLight.direction)), 0.005f);
	float shadow = 0.f;
	// Check whether current fragment is in shadow
	//float closestDepth = texture(u_ShadowMap, projCoord.xy).r;
	//shadow = currentDepth - bias > closestDepth ? 1.f : 0.f;
	// To solve aliasing via PCF
	// textureSize() returns a vec2 of the width and height of the given sampler texture at mipmap level 0
	vec2 texelSize = 1.f / textureSize(u_ShadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float PCFDepth = texture(u_ShadowMap, projCoord.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > PCFDepth ? 1.f : 0.f;
		}
	}
	shadow /= 9.f;

	// To solve over sampling (keep the shadow at 0 when outside the far_plane region of the light's frustum)
	if (projCoord.z > 1.f)
	{
		shadow = 0.f;
	}

	return shadow;
}
