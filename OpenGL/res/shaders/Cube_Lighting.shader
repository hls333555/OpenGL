#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;
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

vec3 CalculateDirectionalLight(DirectionalLight dirLight, vec3 viewDir, vec3 normal);

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 color;

uniform Material u_Material;
uniform DirectionalLight u_DirLight;
uniform vec3 u_ViewPos;

void main()
{
	vec3 directionalColor = CalculateDirectionalLight(u_DirLight, normalize(u_ViewPos - v_FragPos), v_Normal);
	color = vec4(directionalColor, 1.f);
}

vec3 CalculateDirectionalLight(DirectionalLight dirLight, vec3 viewDir, vec3 normal)
{
	vec3 ambientColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * dirLight.ambientIntensity;

	vec3 lightDir = normalize(-dirLight.direction);
	float diff = max(dot(normal, lightDir), 0.f);
	vec3 diffuseColor = vec3(texture(u_Material.diffuseTex, v_TexCoord)) * diff * dirLight.diffuseIntensity;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_Material.shininess);
	vec3 specularColor = vec3(texture(u_Material.specularTex, v_TexCoord)) * spec * dirLight.specularIntensity;

	return ambientColor + diffuseColor + specularColor;
}
