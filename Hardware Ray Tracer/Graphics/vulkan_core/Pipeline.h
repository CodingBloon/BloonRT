#pragma once
#include "Device.h"
#include "Buffer.h"
#include <array>
#include <vector>

namespace Core {
	class Pipeline {

		enum StageIndices {
			eRaygen,
			eMiss,
			eClosestHit,
			eShaderGroupCount
		};

	public:
		Pipeline(Device* device);
		~Pipeline();

	private:
		void createRayTracingPipelineLayout();
		void createRayTracingPipeline();
		void createBindingTable(VkRayTracingPipelineCreateInfoKHR* rtPipelineInfo);

		void createShaderStages(std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount>* shaderStages);
		void createShaderGroups(std::vector<VkRayTracingShaderGroupCreateInfoKHR>* shader_groups);
	private:
		Device* device;
		VkPipeline graphicsPipeline;
		VkPipelineLayout graphicsPipelineLayout;
		std::vector<uint8_t> shaderHandles;

		VkStridedDeviceAddressRegionKHR raygenRegion{};
		VkStridedDeviceAddressRegionKHR missRegion{};
		VkStridedDeviceAddressRegionKHR hitRegion{};
		VkStridedDeviceAddressRegionKHR callableRegion{};
	};	
}