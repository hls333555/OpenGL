#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// In world space
out vec3 v_FragPos;
// In world space
out vec3 v_Normal;
out vec2 v_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
	vec4 worldPos = u_Model * vec4(a_Position, 1.f);
	v_FragPos = worldPos.xyz;
	// TODO: move to CPU for better performance
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;

	gl_Position = u_Projection * u_View * worldPos;
}

#shader fragment
#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gColorSpec;

struct Material
{
	sampler2D diffuseTex1;

	bool bHasSpecular;
	sampler2D specularTex1;
};

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform Material u_Material;

void main()
{
	gPosition = v_FragPos;
	gNormal = normalize(v_Normal);
	gColorSpec.rgb = texture(u_Material.diffuseTex1, v_TexCoord).rgb;
	gColorSpec.a = u_Material.bHasSpecular ? texture(u_Material.specularTex1, v_TexCoord).r : 0.f;
}
