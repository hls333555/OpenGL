#shader vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;

out VS_OUT
{
	vec3 color;
} vs_out;

void main()
{
	gl_Position = vec4(a_Position.x, a_Position.y, 0.f, 1.f);
	vs_out.color = a_Color;
}

#shader geometry
#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 color;
} gs_in[]; // note this should be defined as array

out vec3 v_Color;

#define SIDE_LENGTH 0.2f

void main()
{
	// There is only one vertex since we specify "points" as in-layout, if we specify "triangles", there will be 3 vertices
	vec4 pos = gl_in[0].gl_Position;

	// Build a hexagon
	gl_Position = pos + vec4(-SIDE_LENGTH, 0.f, 0.f, 0.f);
	v_Color = vec3(1.f);
	EmitVertex();
	gl_Position = pos + vec4(-SIDE_LENGTH / 2.f, -SIDE_LENGTH /2.f * sqrt(3.f), 0.f, 0.f);
	v_Color = gs_in[0].color;
	EmitVertex();
	gl_Position = pos + vec4(-SIDE_LENGTH / 2.f, SIDE_LENGTH / 2.f * sqrt(3.f), 0.f, 0.f);
	EmitVertex();
	gl_Position = pos + vec4(SIDE_LENGTH / 2.f, -SIDE_LENGTH / 2.f * sqrt(3.f), 0.f, 0.f);
	EmitVertex();
	gl_Position = pos + vec4(SIDE_LENGTH / 2.f, SIDE_LENGTH / 2.f * sqrt(3.f), 0.f, 0.f);
	EmitVertex();
	gl_Position = pos + vec4(SIDE_LENGTH, 0.f, 0.f, 0.f);
	v_Color = vec3(1.f);
	EmitVertex();
	EndPrimitive();
}

#shader fragment
#version 330 core

in vec3 v_Color;

out vec4 color;

void main()
{
	color = vec4(v_Color, 1.f);
}
