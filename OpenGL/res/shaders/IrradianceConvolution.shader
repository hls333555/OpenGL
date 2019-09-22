#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 v_WorldPos;

uniform mat4 u_ViewProjection;

void main()
{
	v_WorldPos = a_Position;
	gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.f);
}

#shader fragment
#version 330 core

#define PI 3.14159265359

in vec3 v_WorldPos;

out vec4 fragColor;

uniform samplerCube u_EnvironmentMap;

void main()
{
	// The world vector acts as the normal of a tangent surface
	// from the origin, aligned to WorldPos. Given this normal, calculate all
	// incoming radiance of the environment. The result of this radiance
	// is the radiance of light coming from -Normal direction, which is what
	// we use in the PBR shader to sample irradiance.
	vec3 N = normalize(v_WorldPos);

	vec3 irradiance = vec3(0.f);

	// tangent space calculation from origin point
	vec3 up = vec3(0.f, 1.f, 0.f);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025f;
	float numSamples = 0.f;
	for (float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
	{
		for (float theta = 0.f; theta < 0.5f * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			++numSamples;
		}
	}
	irradiance = PI * irradiance * (1.f / numSamples);

	fragColor = vec4(irradiance, 1.f);
}
