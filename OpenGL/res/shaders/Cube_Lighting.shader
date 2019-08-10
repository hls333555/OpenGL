#shader vertex
#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(position, 1.f);
}	

#shader fragment
#version 330 core

out vec4 color;

uniform vec3 u_LightColor;
uniform vec3 u_ObjectColor;

void main()
{
	color = vec4(u_LightColor * u_ObjectColor, 1.f);
}
