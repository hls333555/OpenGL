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

in vec3 v_WorldPos;

out vec4 fragColor;

uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591f, 0.3183f);

vec2 SampleSphericalMap(vec3 v)
{
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5f;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(v_WorldPos));
	vec3 color = texture(u_EquirectangularMap, uv).rgb;

	fragColor = vec4(color, 1.f);
}
