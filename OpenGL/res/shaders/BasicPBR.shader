#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out VS_OUT
{
	vec3 v_WorldPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} vs_out;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main()
{
	vs_out.v_TexCoord = a_TexCoord;
	vs_out.v_WorldPos = vec3(u_Model * vec4(a_Position, 1.f));
	vs_out.v_Normal = mat3(u_Model) * a_Normal;

	gl_Position = u_ViewProjection * vec4(vs_out.v_WorldPos, 1.f);
}

#shader fragment
#version 330 core

#define PI 3.14159265359
#define NUM_POINTLIGHTS 4

in VS_OUT
{
	vec3 v_WorldPos;
	vec3 v_Normal;
	vec2 v_TexCoord;
} fs_in;

out vec4 fragColor;

struct Material
{
	vec3 baseColor;
	float metallic;
	float roughness;
	float ao;
};

struct Material2
{
	sampler2D baseColorMap;
	//sampler2D normalMap;
	sampler2D metallicMap;
	sampler2D roughnessMap;
	//sampler2D aoMap;
	float ao;
};

struct PointLight
{
	vec3 position;
	vec3 color;
};

uniform bool u_bUseTexture;
uniform Material u_Material;
uniform Material2 u_Material2;
uniform PointLight u_PointLights[NUM_POINTLIGHTS];
uniform vec3 u_ViewPos;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	//a = roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	denom = PI * denom * denom;

	// Prevent divide by zero for roughness = 0.f and NdotH = 1.f
	return nom / max(denom, 0.001f);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.f;
	float k = (r * r) / 8.f;

	float nom = NdotV;
	float denom = NdotV * (1.f - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.f);
	float NdotL = max(dot(N, L), 0.f);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.f - F0) * pow(1.f - cosTheta, 5.f);
}

void main()
{
	vec3 baseColor = u_bUseTexture ? pow(texture(u_Material2.baseColorMap, fs_in.v_TexCoord).rgb, vec3(2.2)) : u_Material.baseColor;
	float metallic = u_bUseTexture ? texture(u_Material2.metallicMap, fs_in.v_TexCoord).r : u_Material.metallic;
	float roughness = u_bUseTexture ? texture(u_Material2.roughnessMap, fs_in.v_TexCoord).r : u_Material.roughness;
	float ao = u_bUseTexture ? u_Material2.ao : u_Material.ao;

	vec3 N = normalize(fs_in.v_Normal);
	// vec3 N = getNormalFromMap();
	vec3 V = normalize(u_ViewPos - fs_in.v_WorldPos);

	// Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04f and if it's a metal, use the base color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, baseColor, metallic);

	// Reflectance equation
	vec3 Lo = vec3(0.f);
	int num = u_bUseTexture ? 1 : NUM_POINTLIGHTS;
	for (int i = 0; i < num; ++i)
	{
		// Calculate per-light radiance
		vec3 L = normalize(u_PointLights[i].position - fs_in.v_WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(u_PointLights[i].position - fs_in.v_WorldPos);
		float attenuation = 1.f / (distance * distance);
		vec3 radiance = u_PointLights[i].color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(clamp(dot(H, V), 0.f, 1.f), F0);

		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.f) * max(dot(N, L), 0.f);
		// Prevent divide by zero for NdotV = 0.f or NdotL = 0.f
		vec3 specular = nominator / max(denominator, 0.001f);

		// kS is equal to Fresnel
		vec3 kS = F;
		// For energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.f - kS
		vec3 kD = vec3(1.f) - kS;
		// Multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light)
		kD *= 1.f - metallic;

		// Scale light by NdotL
		float NdotL = max(dot(N, L), 0.f);

		// Add to outgoing radiance Lo
		// Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
		Lo += (kD * baseColor / PI + specular) * radiance * NdotL;
	}

	// Ambient lighting (This ambient lighting will be replaced with environment lighting in the future)
	vec3 ambient = vec3(0.03f) * baseColor * ao;

	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.f));
	// Gamma correction
	color = pow(color, vec3(1.f / 2.2f));

	fragColor = vec4(color, 1.f);
}
