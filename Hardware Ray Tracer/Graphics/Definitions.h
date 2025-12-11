#pragma once

#include "Window.h"
#include <iostream>

#define VK_CHECK_RESULT(f, m) { if(f != VK_SUCCESS) throw std::runtime_error(m); }

/*extern void checkVulkanResult(VkResult result, const char* message) {
	if (result != VK_SUCCESS)
		throw std::runtime_error(message);
}*/
