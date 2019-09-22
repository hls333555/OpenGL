#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out VS_OUT
{
	vec2 v_TexCoord;
	vec3 v_Normal;
	vec3 v_WorldPos;	
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
	vec2 v_TexCoord;
	vec3 v_Normal;
	vec3 v_WorldPos;
} fs_in;

out vec4 fragColor;

struct Material
{
	vec3 baseColor;
	float metallic;
	float roughness;
	float ao;
};

struct PointLight
{
	vec3 position;
	vec3 color;
};

uniform vec3 u_ViewPos;
uniform Material u_Material;
uniform PointLight u_PointLights[NUM_POINTLIGHTS];
// IBL
uniform samplerCube u_IrradianceMap;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main()
{
	vec3 N = fs_in.v_Normal;
	vec3 V = normalize(u_ViewPos - fs_in.v_WorldPos);
	vec3 R = reflect(-V, N);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the baseColor color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, u_Material.baseColor, u_Material.metallic);

	// reflectance equation
	vec3 Lo = vec3(0.f);
	for (int i = 0; i < NUM_POINTLIGHTS; ++i)
	{
		// calculate per-light radiance
		vec3 L = normalize(u_PointLights[i].position - fs_in.v_WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(u_PointLights[i].position - fs_in.v_WorldPos);
		float attenuation = 1.f / (distance * distance);
		vec3 radiance = u_PointLights[i].color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, u_Material.roughness);
		float G = GeometrySmith(N, V, L, u_Material.roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.f), F0);

		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + 0.001f; // 0.001f to prevent divide by zero
		vec3 specular = nominator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.f (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.f - kS.
		vec3 kD = vec3(1.f) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.f - u_Material.metallic;

		// scale light by NdotL
		float NdotL = max(dot(N, L), 0.f);

		// add to outgoing radiance Lo
		Lo += (kD * u_Material.baseColor / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// ambient lighting (we now use IBL as the ambient term)
	vec3 kS = fresnelSchlick(max(dot(N, V), 0.f), F0);
	vec3 kD = 1.f - kS;
	kD *= 1.f - u_Material.metallic;
	vec3 irradiance = texture(u_IrradianceMap, N).rgb;
	vec3 diffuse = irradiance * u_Material.baseColor;
	vec3 ambient = (kD * diffuse) * u_Material.ao;
	//vec3 ambient = vec3(0.002f);

	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.f));
	// Gamma correction
	color = pow(color, vec3(1.f / 2.2f));

	fragColor = vec4(color, 1.f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.f);
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
