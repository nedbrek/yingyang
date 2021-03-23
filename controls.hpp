#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Controls
{
public:
	/// constructor
	explicit Controls(GLFWwindow *window);

	glm::mat4 viewMatrix() const { return view_matrix_; }
	glm::mat4 projectionMatrix() const { return projection_matrix_; }

	void computeMatricesFromInputs();

private:
	static constexpr float SPEED = 3.0f; // 3 units / second
	static constexpr float MOUSE_SPEED = 0.005f;

	glm::vec3 position_ = glm::vec3(4, 3, -3); ///< position : on +Z
	float horizontal_angle_ = 3.14f; ///< horizontal angle : toward -Z
	float   vertical_angle_ = 0.00f; ///< vertical angle : none
	float initial_fov_ = 45.0f; ///< Field of View

	GLFWwindow *window_; ///< window we control
	glm::mat4 view_matrix_; ///< second part of MVP
	glm::mat4 projection_matrix_; ///< first part of MVP
};

