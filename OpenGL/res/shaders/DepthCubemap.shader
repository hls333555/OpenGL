#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;

void main()
{
	gl_Position = u_Model * vec4(a_Position, 1.f);
}

#shader geometry
#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_ShadowMatrices[6];

out vec4 fragPos;

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		// Built-in variable that specifies to which face we render
		gl_Layer = face;
		for (int vertex = 0; vertex < 3; ++vertex)
		{
			fragPos = gl_in[vertex].gl_Position;
			gl_Position = u_ShadowMatrices[face] * fragPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#shader fragment
#version 330 core

in vec4 fragPos;

uniform vec3 u_LightPos;
uniform float u_FarPlane;

void main()
{
	// Get distance between fragment and light source
	float lightDist = distance(fragPos.xyz, u_LightPos);
	// Map to [0,1] range by dividing u_FarPlane
	lightDist /= u_FarPlane;
	// Write this modified depth
	gl_FragDepth = lightDist;
}
