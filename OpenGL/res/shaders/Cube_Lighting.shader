#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
}	

#shader fragment
#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 color;

uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;
uniform vec3 u_ObjectColor;

void main()
{
	float ambientIntensity = 0.1f;
	vec3 ambientColor = ambientIntensity * u_LightColor;

	vec3 lightDir = normalize(u_LightPos - v_FragPos);
	vec3 normalizedNorm = normalize(v_Normal);
	float diffuse = max(dot(normalizedNorm, lightDir), 0.f);
	vec3 diffuseColor = diffuse * u_LightColor;

	float specularIntensity = 0.5f;
	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, normalizedNorm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), 32);
	vec3 specularColor = specularIntensity * spec * u_LightColor;

	color = vec4((ambientColor + diffuseColor + specularColor) * u_ObjectColor, 1.f);
}
