#shader vertex
#version 330 core

// Position vertex attribute data
layout(location = 0) in vec4 position;
// Texture coordinate vertex attribute data
layout(location = 1) in vec2 texCoord;
// Pass texture coordinate out to the fragment shader
out vec2 v_texCoord;

void main()
{
	gl_Position = position;
	v_texCoord = texCoord;
}	

#shader fragment
#version 330 core

// Passed in texture coordinate
in vec2 v_texCoord;
// The output color
layout(location = 0) out vec4 color;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	// Draw the pixel from the texture to the scene by knowing the precise location in the texture to look up
	color = texture(u_Texture, v_texCoord);
}
