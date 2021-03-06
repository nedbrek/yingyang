// needs to be before GL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main(int argc, char **argv)
{
	if (!glfwInit())
	{
		std::cerr << "Failed GLFW init" << std::endl;
		return 1;
	}

	std::cout << "Success!" << std::endl;
	return 0;
}

