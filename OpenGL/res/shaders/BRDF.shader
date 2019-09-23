#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;

	gl_Position = vec4(a_Position, 1.f);
}

#shader fragment
#version 330 core

#define PI 3.14159265359

in vec2 v_TexCoord;

out vec2 fragColor;

float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec2 IntegrateBRDF(float NdotV, float roughness);

void main()
{
	vec2 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y);
	fragColor = integratedBRDF;
}

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// Efficient VanDerCorpus calculation
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.f * PI * Xi.x;
	float cosTheta = sqrt((1.f - Xi.y) / (1.f + (a * a - 1.f) * Xi.y));
	float sinTheta = sqrt(1.f - cosTheta * cosTheta);

	// From spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// From tangent-space H vector to world-space sample vector
	vec3 up = abs(N.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	// Note that we use a different k for IBL
	float a = roughness;
	float k = (a * a) / 2.f;

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

vec2 IntegrateBRDF(float NdotV, float roughness)
{
	vec3 V;
	V.x = sqrt(1.f - NdotV * NdotV);
	V.y = 0.f;
	V.z = NdotV;

	float A = 0.f;
	float B = 0.f;

	vec3 N = vec3(0.f, 0.f, 1.f);

	const uint SAMPLE_COUNT = 1024u;
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		// Generates a sample vector that's biased towards the
		// preferred alignment direction (importance sampling)
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.f * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.f);
		float NdotH = max(H.z, 0.f);
		float VdotH = max(dot(V, H), 0.f);

		if (NdotL > 0.f)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.f - VdotH, 5.f);

			A += (1.f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return vec2(A, B);
}
