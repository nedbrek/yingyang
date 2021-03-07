#version 330

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertex_position_modelspace;
layout(location = 1) in vec3 vertex_color;

// Output data; will be interpolated for each fragment
out vec3 fragment_color;

// uniforms stay constant for the whole mesh
uniform mat4 MVP;

void main()
{
	// pass position (append W as 1)
	gl_Position = MVP * vec4(vertex_position_modelspace, 1);

	// The color of each vertex will be interpolated
	// to produce the color of each fragment
	fragment_color = vertex_color;
}

