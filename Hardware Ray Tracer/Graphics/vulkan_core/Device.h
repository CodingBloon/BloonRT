#pragma once
#include "../Window.h"

namespace Core {

	class Device {
	public:
		Device();
		~Device();

		VkDevice getDevice();
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  getAccelProperties();
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR getRTProperties();
	private:

	};

}