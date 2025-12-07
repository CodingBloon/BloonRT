#include "Pipeline.h"

Core::Pipeline::Pipeline(Device* device) : device(device) {
	createRayTracingPipelineLayout();
	createRayTracingPipeline();
}

Core::Pipeline::~Pipeline() {
	//TODO: Change first nullptr
	vkDestroyPipeline(device->getDevice(), graphicsPipeline, nullptr); //Destorys the rendering pipeline
	vkDestroyPipelineLayout(device->getDevice(), graphicsPipelineLayout, nullptr); //Destroys the rendering pipeline layout
}

void Core::Pipeline::createRayTracingPipelineLayout() {

	VkPushConstantRange push_constant{};

	VkPipelineLayoutCreateInfo creationInfo = {};
	creationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	creationInfo.pushConstantRangeCount = 1;
	creationInfo.pPushConstantRanges = &push_constant;

	std::array<VkDescriptorSetLayout, 2> layouts = {};

	creationInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	creationInfo.pSetLayouts = layouts.data();
	vkCreatePipelineLayout(device->getDevice(), &creationInfo, nullptr, &graphicsPipelineLayout);
}

void Core::Pipeline::createRayTracingPipeline() {
	std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages;
	createShaderStages(&stages);
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	createShaderGroups(&shader_groups);

	VkRayTracingPipelineCreateInfoKHR creationInfo = {};
	creationInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	creationInfo.stageCount = static_cast<uint32_t>(stages.size());
	creationInfo.pStages = stages.data();
	creationInfo.groupCount = static_cast<uint32_t>(shader_groups.size());
	creationInfo.pGroups = shader_groups.data();
	creationInfo.maxPipelineRayRecursionDepth = std::max(3U, device->getRTProperties().maxRayRecursionDepth);
	creationInfo.layout = graphicsPipelineLayout;

	vkCreateRayTracingPipelinesKHR(device->getDevice(), {}, {}, 1, &creationInfo, nullptr, &graphicsPipeline);
}

void Core::Pipeline::createBindingTable(VkRayTracingPipelineCreateInfoKHR* rtPipelineInfo) {
	
	uint32_t groupHandleSize = device->getRTProperties().shaderGroupHandleSize;
	uint32_t groupHandleAlignment = device->getRTProperties().shaderGroupHandleAlignment;
	uint32_t groupBaseAlignment = device->getRTProperties().shaderGroupBaseAlignment;
	uint32_t groupCount = rtPipelineInfo->groupCount;

	size_t dataSize = groupHandleSize / groupCount;
	shaderHandles.resize(dataSize);
	vkGetRayTracingShaderGroupHandlesKHR(device->getDevice(), graphicsPipeline, 0, groupCount, dataSize, &shaderHandles);

	auto alignUp = [](uint32_t size, uint32_t alignment) { return (size + alignment - 1) & ~(alignment - 1); };

	uint32_t raygenShaderSize = alignUp(groupHandleSize, groupHandleAlignment);
	uint32_t missShaderSize = alignUp(groupHandleSize, groupHandleAlignment);
	uint32_t hitShaderSize = alignUp(groupHandleSize, groupHandleAlignment);
	uint32_t callableShaderSize = alignUp(groupHandleSize, groupHandleAlignment);

	uint32_t raygenShaderOffset = 0;
	uint32_t missShaderOffset = alignUp(raygenShaderSize, groupBaseAlignment);
	uint32_t hitShaderOffset = alignUp(missShaderOffset + missShaderSize, groupBaseAlignment);
	uint32_t callableShaderOffset = alignUp(hitShaderOffset + hitShaderSize, groupBaseAlignment);

	size_t bufferSize = callableShaderOffset + callableShaderSize;
	//TODO: Add Method to create buffer
	Buffer buffer = Buffer();


	uint8_t* pData = static_cast<uint8_t*>(buffer.getMapped());
	memcpy(pData + raygenShaderOffset, shaderHandles.data() + 0 * groupHandleSize, groupHandleSize);
	raygenRegion.deviceAddress = buffer.getAddress() + raygenShaderOffset;
	raygenRegion.stride = raygenShaderSize;
	raygenRegion.size = raygenShaderSize;

	memcpy(pData + missShaderOffset, shaderHandles.data() + 1 * groupHandleSize, groupHandleSize);
	missRegion.deviceAddress = buffer.getAddress() + missShaderOffset;
	missRegion.stride = missShaderSize;
	missRegion.size = missShaderSize;

	memcpy(pData + hitShaderOffset, shaderHandles.data() + 2 * groupHandleSize, groupHandleSize);
	hitRegion.deviceAddress = buffer.getAddress() + hitShaderOffset;
	hitRegion.stride = hitShaderSize;
	hitRegion.size = hitShaderSize;

	callableRegion.deviceAddress = 0;
	callableRegion.stride = 0;
	callableRegion.size = 0;
}

void Core::Pipeline::createShaderStages(std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount>* shaderStages) {
	//Ray Generation Shader
	(*shaderStages)[eRaygen].pNext = nullptr; //TODO: Shader code
	(*shaderStages)[eRaygen].pName = "rgenMain"; //Start point for shader
	(*shaderStages)[eRaygen].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	//Miss Shader
	(*shaderStages)[eMiss].pNext = nullptr;
	(*shaderStages)[eMiss].pName = "rmissMain"; //Start point for shader
	(*shaderStages)[eMiss].stage = VK_SHADER_STAGE_MISS_BIT_KHR;

	//Closest Hit Shader
	(*shaderStages)[eClosestHit].pNext = nullptr;
	(*shaderStages)[eClosestHit].pName = "chitMain";
	(*shaderStages)[eMiss].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
}

void Core::Pipeline::createShaderGroups(std::vector<VkRayTracingShaderGroupCreateInfoKHR>* shader_groups) {
	VkRayTracingShaderGroupCreateInfoKHR group;
	group.anyHitShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = VK_SHADER_UNUSED_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.intersectionShader = VK_SHADER_UNUSED_KHR;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = eRaygen;
	(*shader_groups).push_back(group);

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = eMiss;
	(*shader_groups).push_back(group);

	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = eClosestHit;
	(*shader_groups).push_back(group);
}
