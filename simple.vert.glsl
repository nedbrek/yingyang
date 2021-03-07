#version 330

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertex_position_modelspace;

// uniforms stay constant for the whole mesh
uniform mat4 MVP;

void main()
{
	// pass position (append W as 1)
	gl_Position = MVP * vec4(vertex_position_modelspace, 1);
}

