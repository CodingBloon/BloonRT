#include "Camera.h"
#include <cassert>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

void Core::Camera::setPerspectiveProjection(float fovy, float aspectRatio, float near, float far) {
	assert(glm::abs(aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f);
	const float tanHalfFovy = tan(fovy / 2.f);
	projection = glm::mat4{ 0.0f };
	projection = glm::mat4{ 0.0f };
	projection[0][0] = 1.f / (aspectRatio * tanHalfFovy);
	projection[1][1] = 1.f / (tanHalfFovy);
	projection[2][2] = far / (far - near);
	projection[2][3] = 1.f;
	projection[3][2] = -(far * near) / (far - near);
}

void Core::Camera::setView(glm::vec3 position, glm::vec3 rotation) {
	this->position = position;
	this->rotation = rotation;

	updateView();
}

void Core::Camera::handleInputs(GLFWwindow* window, float dt) {
	glm::vec3 rotate{ 0.0f };

	if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
	if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;

	if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
	if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		rotation += 1.5f * dt * glm::normalize(rotate);

	rotation.x = glm::clamp(rotation.x, -1.5f, 1.5f);
	rotation.y = glm::mod(rotation.y, glm::two_pi<float>());

	float yaw = rotation.y;
	const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
	const glm::vec3 upDir{ 0.0f, -1.0f, 0.0f };

	glm::vec3 moveDir{ 0.0f };

	if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
	if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;

	if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
	if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;

	if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
	if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		position += 3.f * dt * glm::normalize(moveDir);

	updateView();
}

glm::mat4 Core::Camera::getProjection() {
	return projection;
}

glm::mat4 Core::Camera::getView() {
	return view;
}

void Core::Camera::updateView() {
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	view = glm::mat4{ 1.f };
	view[0][0] = u.x;
	view[1][0] = u.y;
	view[2][0] = u.z;
	view[0][1] = v.x;
	view[1][1] = v.y;
	view[2][1] = v.z;
	view[0][2] = w.x;
	view[1][2] = w.y;
	view[2][2] = w.z;
	view[3][0] = -glm::dot(u, position);
	view[3][1] = -glm::dot(v, position);
	view[3][2] = -glm::dot(w, position);
}