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

	// create main window
	GLFWwindow *window = glfwCreateWindow( 1024, 768, "YingYang", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 2;
	}

	// make sure we get all key presses
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// background color (blue .4)
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	do
	{
		// erase screen before drawing
		glClear(GL_COLOR_BUFFER_BIT);

		// here is where we would draw

		// done! swap buffer to front
		glfwSwapBuffers(window);
		glfwPollEvents(); // get events

		// while not escape key, or close window button
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	         glfwWindowShouldClose(window) == 0);

	glfwTerminate();
	return 0;
}

