#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 v_WorldPos;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	v_WorldPos = a_Position;

	mat4 rotView = mat4(mat3(u_View));
	vec4 clipPos = u_Projection * rotView * vec4(v_WorldPos, 1.f);

	gl_Position = clipPos.xyww;
}

#shader fragment
#version 330 core

in vec3 v_WorldPos;

out vec4 fragColor;

uniform samplerCube u_EnvironmentMap;

void main()
{
	vec3 envColor = texture(u_EnvironmentMap, v_WorldPos).rgb;

	// HDR tonemapping and gamma correction
	envColor = envColor / (envColor + vec3(1.f));
	envColor = pow(envColor, vec3(1.f / 2.2f));

	fragColor = vec4(envColor, 1.f);
}
