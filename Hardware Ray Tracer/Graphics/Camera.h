#pragma once

#include "Window.h"
#include <glm/glm.hpp>

namespace Core {

	class Camera {
	public:
		void setPerspectiveProjection(float fovy, float aspectRation, float near, float far);
		void setView(glm::vec3 position, glm::vec3 rotation);

		void handleInputs(GLFWwindow* window, float dt);
		//void handleMouseInputs();

		glm::mat4 getProjection();
		glm::mat4 getView();

	private:
		void updateView();
	private:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookRight = GLFW_KEY_RIGHT;
			int lookLeft = GLFW_KEY_LEFT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
			int mouseDown = GLFW_MOUSE_BUTTON_2;
		};
	private:
		glm::vec3 position;
		glm::vec3 rotation;

		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };

		KeyMappings keys{};
	};

}