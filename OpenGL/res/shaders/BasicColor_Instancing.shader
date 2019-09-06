#shader vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_Offset;

out vec3 v_Color;

void main()
{
	gl_Position = vec4(a_Position + a_Offset, 0.f, 1.f);
	v_Color = a_Color;
}

#shader fragment
#version 330 core

in vec3 v_Color;

out vec4 color;

void main()
{
	color = vec4(v_Color, 1.f);
}
