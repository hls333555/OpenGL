#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
// Normals take the place of location 1
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in mat4 a_InstancedMatrix;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * a_InstancedMatrix * vec4(a_Position, 1.f);
	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;

out vec4 color;

uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_TexCoord);
}
