#shader vertex
#version 330 core

// Position vertex attribute data
layout(location = 0) in vec2 a_Position;
// Texture coordinate vertex attribute data
layout(location = 1) in vec2 a_TexCoord;

// Pass texture coordinate out to the fragment shader
out vec2 v_TexCoord;

void main()
{
	gl_Position = vec4(a_Position.x, a_Position.y, 0.f, 1.f);
	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

// Passed in texture coordinate
in vec2 v_TexCoord;

// The output color
out vec4 color;

uniform sampler2D u_ScreenTexture;

vec4 Inversion(vec4 color);
vec4 Greyscale(vec4 color);
vec4 Kernel(float kernel[9], sampler2D texture, vec2 texCoord);

void main()
{
	vec4 defaultColor = texture(u_ScreenTexture, v_TexCoord);

	//color = Inversion(defaultColor);
	//color = Greyscale(defaultColor);

	float sharpenKernel[9] = {
		-1.f, -1.f, -1.f,
		-1.f,  9.f, -1.f,
		-1.f, -1.f, -1.f
	};
	float blurKernel[9] = {
		1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
		2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
		1.f / 16.f, 2.f / 16.f, 1.f / 16.f
	};
	float edgeDetectionKernel[9] = {
		1.f,  1.f, 1.f,
		1.f, -8.f, 1.f,
		1.f,  1.f, 1.f
	};
	color = Kernel(edgeDetectionKernel, u_ScreenTexture, v_TexCoord);
}

vec4 Inversion(vec4 color)
{
	return vec4(vec3(1.f - color), 1.f);
}

vec4 Greyscale(vec4 color)
{
	//float average = (color.r + color.g + color.b) / 3.f;
	float average = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
	return vec4(vec3(average), 1.f);
}

vec4 Kernel(float kernel[9], sampler2D texture, vec2 texCoord)
{
	const float offset = 1.f / 300.f;

	vec2 offsets[9] = {
		vec2(-offset, offset),  vec2(0.f, offset),  vec2(offset, offset),
		vec2(-offset, 0.f),     vec2(0.f, 0.f),     vec2(offset, 0.f),
		vec2(-offset, -offset), vec2(0.f, -offset), vec2(offset, offset)
	};

	vec3 sampleTex[9];
	for (int i = 0; i < 9; ++i)
	{
		sampleTex[i] = vec3(texture(texture, texCoord + offsets[i]));
	}

	vec3 color = vec3(0.f);
	for (int i = 0; i < 9; ++i)
	{
		color += sampleTex[i] * kernel[i];
	}
	return vec4(color, 1.f);
}
