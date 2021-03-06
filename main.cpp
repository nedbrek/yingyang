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
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW init failed" << std::endl;
		glfwTerminate();
		return 3;
	}

	// make sure we get all key presses
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// background color (blue .4)
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// main vertex array
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);

	// put a triangle in the vertex buffer
	const GLfloat vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
	};
	glBindVertexArray(vertex_array_id);

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	do
	{
		// erase screen before drawing
		glClear(GL_COLOR_BUFFER_BIT);

		// here is where we draw
		//
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangle!
		glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle

		// done with that attribute
		glDisableVertexAttribArray(0);

		// done drawing! swap buffer to front
		glfwSwapBuffers(window);
		glfwPollEvents(); // get events

		// while not escape key, or close window button
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	         glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteVertexArrays(1, &vertex_array_id);

	glfwTerminate();
	return 0;
}

