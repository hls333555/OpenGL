#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

out VS_OUT
{
	vec3 clipSpaceNormal;
} vs_out;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.f);

	mat3 normalMatrix = mat3(transpose(inverse(u_View * u_Model)));
	// GS receives its position vectors as clip-space coordinates
	// so we should also transform the normal vectors to the same space
	vs_out.clipSpaceNormal = normalize(vec3(u_Projection * vec4(normalMatrix * a_Normal, 1.f)));
}	

#shader geometry
#version 330 core

layout(triangles) in;
// Draw three lines as normals for three vertices respectively for one triangle
layout(line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 clipSpaceNormal;
} gs_in[];

#define NORMAL_MAG 0.1f

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = gl_in[i].gl_Position + vec4(gs_in[i].clipSpaceNormal, 0.f) * NORMAL_MAG;
		EmitVertex();
		EndPrimitive();
	}
}

#shader fragment
#version 330 core

out vec4 color;

void main()
{
	color = vec4(0.f, 1.f, 0.f, 1.f);
}
