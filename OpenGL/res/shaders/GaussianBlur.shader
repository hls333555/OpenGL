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

// Bright color texture
uniform sampler2D u_BrightScene;
uniform bool u_bHorizontal;
// Gaussian weights
uniform float weights[5] = {
	0.2270270270f,
	0.1945945946f,
	0.1216216216f,
	0.0540540541f,
	0.0162162162f
};

void main()
{
	vec2 texSize = 1.f / textureSize(u_BrightScene, 0);
	vec3 blurColor = texture(u_BrightScene, v_TexCoord).rgb * weights[0];
	if (u_bHorizontal)
	{
		// Blur horizontally ONCE (blending horizontally nearby 4+4 fragments)
		for (int i = 1; i < 5; ++i)
		{
			blurColor += texture(u_BrightScene, v_TexCoord + vec2(texSize.x * i, 0.f)).rgb * weights[i];
			blurColor += texture(u_BrightScene, v_TexCoord - vec2(texSize.x * i, 0.f)).rgb * weights[i];
		}
	}
	else
	{
		// Blur vertically ONCE (blending vertically nearby 4+4 fragments)
		for (int i = 1; i < 5; ++i)
		{
			blurColor += texture(u_BrightScene, v_TexCoord + vec2(0.f, texSize.y * i)).rgb * weights[i];
			blurColor += texture(u_BrightScene, v_TexCoord - vec2(0.f, texSize.y * i)).rgb * weights[i];
		}
	}
	
	color = vec4(blurColor, 1.f);
}
