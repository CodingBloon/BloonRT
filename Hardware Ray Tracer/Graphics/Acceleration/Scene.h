#pragma once

#include "../vulkan_core/Device.h"
#include "../vulkan_core/Buffer.h"
#include <vector>

#define vkCmdBuildAccelerationStructuresKHR reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCmdBuildAccelerationStructuresKHR"))
#define vkCreateAccelerationStructureKHR reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCreateAccelerationStructureKHR"))
#define vkGetAccelerationStructureBuildSizesKHR reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkGetAccelerationStructureBuildSizesKHR"))
#define vkDestroyAccelerationStructureKHR reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkDestroyAccelerationStructureKHR"))

namespace Core {
	struct Vertex {
		float position[3];
	};

	struct AccelerationStructure {
		VkAccelerationStructureKHR handle;
		VkDeviceAddress address;
		VkBuffer buffer;
		VkDeviceMemory memory;
	};

	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Buffer& vertexBuffer;
		Buffer& indexBuffer;
	};

	class Scene {
	public:
		Scene(Device& device, std::vector<Mesh> meshes);
		~Scene();
	private:

		void primitiveToGeometry(Mesh& mesh, VkAccelerationStructureGeometryKHR& geometry, VkAccelerationStructureBuildRangeInfoKHR& range);
		void createAccelerationStructure(
			VkAccelerationStructureTypeKHR asType,
			AccelerationStructure& AS,
			VkAccelerationStructureGeometryKHR& geometry,
			VkAccelerationStructureBuildRangeInfoKHR& range,
			VkBuildAccelerationStructureFlagsKHR flags
		);
		void createBottomLevelAs();
		void createTopLevelAs();
		void createAcceleration(AccelerationStructure& structure, VkAccelerationStructureCreateInfoKHR info);
	private:
		Device& device;
		std::vector<Mesh> meshes;
		std::vector<AccelerationStructure> m_blasAccel;
		AccelerationStructure m_tlasAccel;
	};
}