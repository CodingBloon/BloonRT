#pragma once
#include "Device.h"
#include "Buffer.h"
#include "../Acceleration/Scene.h"
#include "SwapChain.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>

#define vkCreateRayTracingPipelinesKHR reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device.getDevice(), "vkCreateRayTracingPipelinesKHR"))

/*namespace Core {
	struct UniformBuffer {
		glm::mat4 viewInverse;
		glm::mat4 projInverse;
	};

	struct ShaderBindingTable {
		Buffer* raygen;
		Buffer* miss;
		Buffer* hit;
	};

	struct StorageImage {
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};

	class Pipeline {

		enum StageIndices {
			eRaygen,
			eMiss,
			eClosestHit,
			eShaderGroupCount
		};

	public:
		Pipeline(Device& device, Scene& scene, SwapChain& swapChain);
		~Pipeline();

	private:
		void createDescriptorSets();
		void createRayTracingPipelineLayout();
		void createRayTracingPipeline();

		void createShaderStages(std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount>* shaderStages);
		void createShaderGroups(std::vector<VkRayTracingShaderGroupCreateInfoKHR>* shader_groups);
		void createShaderBindingTable(ShaderBindingTable* bindingTable, uint32_t groupCount);
		void createStorageImages();

		std::vector<char> readFile(const std::string& filename);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	private:
		Device& device;
		VkPipeline graphicsPipeline;
		VkPipelineLayout graphicsPipelineLayout;
		Scene& scene;
		SwapChain& swapChain;
		ShaderBindingTable bindingTable;
		std::vector<uint8_t> shaderHandles;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSet;
		VkDescriptorSetLayout descriptorSetLayout;
		Buffer uniformBuffer{device, sizeof(UniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		std::vector<StorageImage> storageImages;

		VkShaderModule raygenShaderModule;
		VkShaderModule missShaderModule;
		VkShaderModule closestHitShaderModule;
		//VkImageView outputImageView;
	};	
}*/