#include "controls.hpp"

// Include GLM
#include <glm/gtc/matrix_transform.hpp>

Controls::Controls(GLFWwindow *window)
: window_(window)
{
}

void Controls::computeMatricesFromInputs()
{
	// store last call time
	static double last_time = glfwGetTime();

	// compute time difference between current and last frame
	const double current_time = glfwGetTime();
	const float delta_time = current_time - last_time; // only float used in calculations

	// get mouse position
	double xpos = 0, ypos = 0;
	glfwGetCursorPos(window_, &xpos, &ypos);

	// reset mouse position for next frame
	glfwSetCursorPos(window_, 1024/2, 768/2);

	// compute new orientation
	horizontal_angle_ += MOUSE_SPEED * float(1024/2 - xpos);
	vertical_angle_   += MOUSE_SPEED * float( 768/2 - ypos);

	// direction: Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
	    cosf(vertical_angle_) * sinf(horizontal_angle_),
	    sinf(vertical_angle_),
	    cosf(vertical_angle_) * cosf(horizontal_angle_)
	);

	// right vector
	glm::vec3 right = glm::vec3(
		sinf(horizontal_angle_ - 3.14f/2.0f),
		0,
		cosf(horizontal_angle_ - 3.14f/2.0f)
	);

	// up vector
	glm::vec3 up = glm::cross(right, direction);

	// move forward
	if (glfwGetKey(window_, GLFW_KEY_UP   ) == GLFW_PRESS) {position_ += direction * delta_time * SPEED;}
	// move backward
	if (glfwGetKey(window_, GLFW_KEY_DOWN ) == GLFW_PRESS) {position_ -= direction * delta_time * SPEED;}
	// strafe right
	if (glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS) { position_ += right * delta_time * SPEED; }
	// strafe left
	if (glfwGetKey(window_, GLFW_KEY_LEFT ) == GLFW_PRESS) { position_ -= right * delta_time * SPEED; }
	// fly up
	if (glfwGetKey(window_, GLFW_KEY_PAGE_UP) == GLFW_PRESS) { position_[1] += delta_time * SPEED; }
	// fly down
	if (glfwGetKey(window_, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) { position_[1] -= delta_time * SPEED; }

	// - 5 * glfwGetMouseWheel();
	// Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
	float fov = initial_fov_;

	// projection matrix: 45 deg Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	projection_matrix_ = glm::perspective(glm::radians(fov), 4.0f / 3.0f, 0.1f, 100.0f);
	// camera matrix
	view_matrix_       = glm::lookAt(
	                     position_,           // Camera is here
	                     position_+direction, // and looks here : at the same position, plus "direction"
	                     up                   // Head is up (set to 0,-1,0 to look upside-down)
	                  );

	// for the next frame, the "last time" will be "now"
	last_time = current_time;
}

