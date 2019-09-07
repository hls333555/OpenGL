#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

struct Material
{
	sampler2D diffuseTex1;
};

in vec2 v_TexCoord;

out vec4 color;

uniform Material u_Material;

void main()
{
	color = texture(u_Material.diffuseTex1, v_TexCoord);
}
