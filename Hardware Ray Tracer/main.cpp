#include <iostream>
#include "Graphics/Window.h";
#include "Graphics/vulkan_core/Device.h"
#include "Graphics/Denoiser/Denoiser.h"
#include "Graphics/vulkan_core/Pipeline.h"
#include "Graphics/Acceleration/Scene.h"
#include "Graphics/vulkan_core/Buffer.h"


int main() {
	std::cout << "Hello World!" << std::endl;

	Core::Window window = Core::Window(800, 600, "Vulkan Ray Tracer"); //create window
	Core::Device device{&window}; // = Core::Device(&window); //create vulkan device

	std::vector<Core::Vertex> vertices = {
		{{1.0f, 1.0f, 0.0f}},
		{{-1.0f, 1.0f, 0.0f}},
		{{0.0f, -1.0f, 0.0f}}
	};

	std::vector<uint32_t> indices = {
		0, 1, 2
	};

	Core::Buffer vertexBuffer{device,
		vertices.size() * sizeof(Core::Vertex), 
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR , 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
	Core::Buffer indexBuffer{
		device,
		indices.size() * sizeof(uint32_t),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	vertexBuffer.map();
	vertexBuffer.writeToBuffer(vertices.data(), vertices.size() * sizeof(Core::Vertex), 0);
	indexBuffer.map();
	indexBuffer.writeToBuffer(indices.data(), indices.size() * sizeof(uint32_t), 0);

	Core::Mesh mesh{
		vertices,
		indices,
		vertexBuffer,
		indexBuffer
	};

	Core::Scene scene{ device, std::vector<Core::Mesh>{mesh} }; //create scene with a single mesh*/

	//Core::Pipeline pipeline{ &device }; //create ray tracing pipeline

	while (!window.shouldClose()) {
		glfwPollEvents();
	}
}