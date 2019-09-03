#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

out vec3 v_TexCoord;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.f);
	// Make sure the cube is placed at the origin so that the vertex position can represent the direction vector
	v_TexCoord = a_Position;
}

#shader fragment
#version 330 core

in vec3 v_TexCoord;

out vec4 color;

uniform samplerCube u_CubemapTexture;

void main()
{
	color = texture(u_CubemapTexture, v_TexCoord);
}
