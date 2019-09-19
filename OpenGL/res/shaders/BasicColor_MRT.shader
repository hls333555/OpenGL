#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
}

#shader fragment
#version 330 core

// MRT
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 brightColor;

uniform vec3 u_Color;

void main()
{
	// Calculate the brightness by transforming it to greyscale
	float brightness = dot(u_Color, vec3(0.2126f, 0.7152f, 0.0722f));
	if (brightness > 1.f)
	{
		brightColor = vec4(u_Color, 1.f);
	}
	else
	{
		brightColor = vec4(0.f, 0.f, 0.f, 1.f);
	}

	fragColor = vec4(u_Color, 1.f);
}
