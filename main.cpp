// needs to be before GL
#include <GL/glew.h>
#include "controls.hpp"
#include "loadBmp.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

GLuint loadShaders(const std::string &vertex_file_path, const std::string &fragment_file_path)
{
	// open vertex shader code file
	std::ifstream vertex_shader_stream(vertex_file_path.c_str(), std::ios::in);
	if (!vertex_shader_stream.is_open())
	{
		std::cerr << "Failed to open vertex shader " << vertex_file_path << std::endl;
		return 0;
	}

	// open fragment shader code file
	std::ifstream fragment_shader_stream(fragment_file_path.c_str(), std::ios::in);
	if (!fragment_shader_stream.is_open())
	{
		std::cerr << "Failed to open fragment shader " << fragment_file_path << std::endl;
		return 0;
	}

	// read vertex shader code from file
	std::stringstream vstr;
	vstr << vertex_shader_stream.rdbuf();

	std::string vertex_shader_text = vstr.str();

	vertex_shader_stream.close();

	// read fragment shader code from file
	std::stringstream fstr;
	fstr << fragment_shader_stream.rdbuf();

	std::string fragment_shader_text = fstr.str();

	fragment_shader_stream.close();

	// compile vertex shader
	std::cout << "Compiling vertex shader: " << vertex_file_path << std::endl;
	const char *const vertex_source_pointer = vertex_shader_text.c_str();

	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, NULL);
	glCompileShader(vertex_shader_id);

	//--- check compile result of vertex shader
	GLint result = GL_FALSE;
	int info_log_length = 0;
	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0)
	{
		std::vector<char> vertex_shader_error_message(info_log_length+1);
		glGetShaderInfoLog(vertex_shader_id, info_log_length, NULL, &vertex_shader_error_message[0]);
		std::cerr << &vertex_shader_error_message[0] << std::endl;
	}
	if (result != GL_TRUE)
	{
		std::cerr << "Vertex shader compile gave result: " << result << std::endl;
		return 0;
	}

	// compile fragment shader
	std::cout << "Compiling fragment shader: " << fragment_file_path << std::endl;
	const char *const fragment_source_pointer = fragment_shader_text.c_str();

	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, NULL);
	glCompileShader(fragment_shader_id);

	//--- check compile result of fragment shader
	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0)
	{
		std::vector<char> fragment_shader_error_message(info_log_length+1);
		glGetShaderInfoLog(fragment_shader_id, info_log_length, NULL, &fragment_shader_error_message[0]);
		std::cerr << &fragment_shader_error_message[0] << std::endl;
	}
	if (result != GL_TRUE)
	{
		std::cerr << "Fragment shader compile gave result: " << result << std::endl;
		return 0;
	}

	// link program
	std::cout << "Linking program" << std::endl;
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	// check program
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0)
	{
		std::vector<char> program_error_message(info_log_length+1);
		glGetProgramInfoLog(program_id, info_log_length, NULL, &program_error_message[0]);
		std::cerr << &program_error_message[0] << std::endl;
	}
	if (result != GL_TRUE)
	{
		std::cerr << "Linker gave result: " << result << std::endl;
		return 0;
	}

	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}

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
	Controls *controls = new Controls(window);

	// make sure we get all key presses
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// hide mouse cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// background color (blue .4)
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// enable depth test
	glEnable(GL_DEPTH_TEST);
	// accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// main vertex array
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);

	// put a triangle in the vertex buffer
	// Vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	const GLfloat vertex_buffer_data[36*3] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f
	};
	glBindVertexArray(vertex_array_id);

	// read and compile shaders
	GLuint program_id = loadShaders("../simple.vert.glsl", "../texture.frag.glsl");

	// get a handle for our "MVP" uniform
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");

	// load the texture using any two methods
	GLuint texture = loadBmp("../uvtemplate.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");

	// get a handle for our texture sampler uniform
	GLuint TextureID  = glGetUniformLocation(program_id, "myTextureSampler");

	// Two UV coordinatesfor each vertex. They were created with Blender.
	const GLfloat g_uv_buffer_data[36*2] = {
		0.000059f, 1.0f-0.000004f,
		0.000103f, 1.0f-0.336048f,
		0.335973f, 1.0f-0.335903f,
		1.000023f, 1.0f-0.000013f,
		0.667979f, 1.0f-0.335851f,
		0.999958f, 1.0f-0.336064f,
		0.667979f, 1.0f-0.335851f,
		0.336024f, 1.0f-0.671877f,
		0.667969f, 1.0f-0.671889f,
		1.000023f, 1.0f-0.000013f,
		0.668104f, 1.0f-0.000013f,
		0.667979f, 1.0f-0.335851f,
		0.000059f, 1.0f-0.000004f,
		0.335973f, 1.0f-0.335903f,
		0.336098f, 1.0f-0.000071f,
		0.667979f, 1.0f-0.335851f,
		0.335973f, 1.0f-0.335903f,
		0.336024f, 1.0f-0.671877f,
		1.000004f, 1.0f-0.671847f,
		0.999958f, 1.0f-0.336064f,
		0.667979f, 1.0f-0.335851f,
		0.668104f, 1.0f-0.000013f,
		0.335973f, 1.0f-0.335903f,
		0.667979f, 1.0f-0.335851f,
		0.335973f, 1.0f-0.335903f,
		0.668104f, 1.0f-0.000013f,
		0.336098f, 1.0f-0.000071f,
		0.000103f, 1.0f-0.336048f,
		0.000004f, 1.0f-0.671870f,
		0.336024f, 1.0f-0.671877f,
		0.000103f, 1.0f-0.336048f,
		0.336024f, 1.0f-0.671877f,
		0.335973f, 1.0f-0.335903f,
		0.667969f, 1.0f-0.671889f,
		1.000004f, 1.0f-0.671847f,
		0.667979f, 1.0f-0.335851f
	};

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	std::vector<unsigned> index_data;
	const unsigned vertex_buffer_data_size = sizeof(vertex_buffer_data) / sizeof(vertex_buffer_data[0]);
	for (unsigned i = 0; i < vertex_buffer_data_size/3; ++i)
	{
		index_data.push_back(i);
	}

	GLuint idx_buffer;
	glGenBuffers(1, &idx_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data[0])*index_data.size(), &index_data[0], GL_STATIC_DRAW);

	do
	{
		// erase screen before drawing
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program_id);

		// Compute the MVP matrix from keyboard and mouse input
		controls->computeMatricesFromInputs();
		glm::mat4 projection_matrix = controls->projectionMatrix();
		glm::mat4 view_matrix = controls->viewMatrix();
		glm::mat4 model_matrix = glm::mat4(1.0);
		glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

		// set model view projection matrix
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

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

		// 2nd attribute buffer: colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangles!
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buffer);
		glDrawElements(GL_TRIANGLES, index_data.size(), GL_UNSIGNED_INT, (void*)0);

		// done with that attribute
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// done drawing! swap buffer to front
		glfwSwapBuffers(window);
		glfwPollEvents(); // get events

		// while not escape key, or close window button
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	         glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &vertex_array_id);
	glDeleteProgram(program_id);

	delete controls;
	glfwTerminate();
	return 0;
}

