#include <iostream>
#include "Graphics/Window.h";
#include "Graphics/vulkan_core/Device.h"
#include "Graphics/Denoiser/Denoiser.h"

int main() {
	std::cout << "Hello World!" << std::endl;

	Core::Window window = Core::Window(800, 600, "Vulkan Ray Tracer");

	while (!window.shouldClose()) {
		glfwPollEvents();
	}
}