#shader vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.f);
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.f));
	// TODO: move to CPU for better performance
	// This calculation can ensure normals are transformed properly even if the geometry has applied a non-uiform scale
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
}

#shader fragment
#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 color;

uniform vec3 u_ViewPos;
uniform samplerCube u_CubemapTexture;

void main()
{
	vec3 viewDir = normalize(v_FragPos - u_ViewPos);

	vec3 reflectDir = reflect(viewDir, normalize(v_Normal));

	// Assume light goes from air to glass
	float refractRatio = 1.f / 1.52f;
	vec3 refractDir = refract(viewDir, normalize(v_Normal), refractRatio);

	//color = texture(u_CubemapTexture, reflectDir);
	color = texture(u_CubemapTexture, refractDir);
}
