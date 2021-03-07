#version 330

// Interpolated values from the vertex shaders
in vec3 fragment_color;

// Ouput data
out vec3 color;

void main()
{
	// fixed color, red
	//color = vec3(1, 0, 0);
	// pass input
	color = fragment_color;
}

