#shader vertex
#version 330 core

// Position vertex attribute data
layout(location = 0) in vec4 a_Position;
// Texture coordinate vertex attribute data
layout(location = 1) in vec2 a_TexCoord;
// Pass texture coordinate out to the fragment shader
out vec2 v_TexCoord;

uniform mat4 u_MVP;

void main()
{
	gl_Position =  u_MVP * a_Position;
	v_TexCoord = a_TexCoord;
}	

#shader fragment
#version 330 core

// Passed in texture coordinate
in vec2 v_TexCoord;
// The output color
layout(location = 0) out vec4 color;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	// Draw the pixel from the texture to the scene by knowing the precise location in the texture to look up
	color = texture(u_Texture, v_TexCoord);
}
