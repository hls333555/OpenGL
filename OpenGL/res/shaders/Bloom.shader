#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.f);
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;

out vec4 color;

uniform sampler2D u_NormalScene;
uniform sampler2D u_BlurredScene;
uniform bool u_bEnableBloom;
uniform float u_Exposure;

void main()
{
	const float gamma = 2.2f;
	vec3 hdrColor = texture(u_NormalScene, v_TexCoord).rgb;
	vec3 blurredColor = texture(u_BlurredScene, v_TexCoord).rgb;
	if (u_bEnableBloom)
	{
		// Additive blending
		hdrColor += blurredColor;
	}
	// Tone mapping
	vec3 bloomedColor = vec3(1.f) - exp(-hdrColor * u_Exposure);
	// Gamma correction at last
	bloomedColor = pow(bloomedColor, vec3(1.f / gamma));
	color = vec4(bloomedColor, 1.f);
}
