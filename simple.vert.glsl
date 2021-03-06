#version 330

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertex_position_modelspace;

void main()
{
	// pass position
	gl_Position.xyz = vertex_position_modelspace;
	gl_Position.w = 1.0;
}

