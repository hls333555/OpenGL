#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_Color = vec4(a_Position, 1.f);
	v_TexCoord = a_TexCoord;
}	

#shader fragment
#version 330 core

in vec4 v_Color;
in vec2 v_TexCoord;

out vec4 color;

uniform sampler2D u_Texture;

void main()
{
	color = v_Color;
	//color = texture(u_Texture, v_TexCoord);
}
