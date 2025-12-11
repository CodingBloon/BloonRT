#include "Pipeline.h"

/*
 * Ray Tracing Pipeline has to return depths and normals for each pixel. 
 * This is required for the denoising process. 
 * The denoise process uses the depth and normal information to better understand the scene geometry and improve the quality of the denoised image.
 */

Core::Pipeline::Pipeline(Device& device) : device(device) {
	createRayTracingPipelineLayout();
	createDescriptorSets();
	createRayTracingPipeline();
}

Core::Pipeline::~Pipeline() {
	vkDestroyPipeline(device.getDevice(), graphicsPipeline, nullptr); //Destorys the rendering pipeline
	vkDestroyPipelineLayout(device.getDevice(), graphicsPipelineLayout, nullptr); //Destroys the rendering pipeline layout
}

void Core::Pipeline::createDescriptorSets() {

	std::vector<VkDescriptorPoolSize> poolSizes = {
	{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
	}; //Descriptor pool sizes

	//Create Descriptor Pool
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 3;
	VK_CHECK_RESULT(vkCreateDescriptorPool(device.getDevice(), &poolInfo, nullptr, &descriptorPool), "failed to create descriptor pool");

	//Allocate Descriptor Set
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &descriptorSet), "failed to allocate memory for descriptor sets");


	VkWriteDescriptorSetAccelerationStructureKHR accelInfo = {};
	accelInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	accelInfo.accelerationStructureCount = 0; //Number of acceleration structures (currently 0)
	accelInfo.pAccelerationStructures = nullptr; //Pointer to acceleration structure (currently null)

	//link acceleration structure to descriptor set
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0; //will have index 0 in shader (binding = 0)
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptorWrite.pNext = &accelInfo;

	VkDescriptorImageInfo outputImageInfo = {};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = outputImageView;

	VkWriteDescriptorSet outputImageWrite = {};
	outputImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputImageWrite.dstSet = descriptorSet;
	outputImageWrite.dstBinding = 1; //will have index 1 in shader (binding = 1)
	outputImageWrite.descriptorCount = 1;
	outputImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	outputImageInfo.imageView = outputImageView;
	outputImageWrite.pImageInfo = &outputImageInfo;

	/*VkDescriptorBufferInfo uniformBufferInfo = {};
	uniformBufferInfo.buffer = uniformBuffer.getBuffer();
	uniformBufferInfo.offset = 0; //no offset
	uniformBufferInfo.range = VK_WHOLE_SIZE;*/

	VkDescriptorBufferInfo uniformBufferInfo = uniformBuffer.descriptorInfo();

	VkWriteDescriptorSet uniformBufferWrite = {};
	uniformBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformBufferWrite.dstSet = descriptorSet;
	uniformBufferWrite.dstBinding = 2; //will have index 2 in shader (binding = 2)
	uniformBufferWrite.descriptorCount = 1;
	uniformBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferWrite.pBufferInfo = &uniformBufferInfo; //Pointer to uniform buffer info

	std::vector<VkWriteDescriptorSet> writeDescriptors = {
		descriptorWrite,
		outputImageWrite,
		uniformBufferWrite
	};

	//Update Descriptor Sets
	vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
}

void Core::Pipeline::createRayTracingPipelineLayout() {

	VkDescriptorSetLayoutBinding accelLayoutBinding = {};
	accelLayoutBinding.binding = 0;
	accelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelLayoutBinding.descriptorCount = 1;
	accelLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding storageImageLayoutBinding = {};
	storageImageLayoutBinding.binding = 1;
	storageImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	storageImageLayoutBinding.descriptorCount = 1;
	storageImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniformBufferLayoutBinding = {};
	uniformBufferLayoutBinding.binding = 2;
	uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferLayoutBinding.descriptorCount = 1;
	uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		accelLayoutBinding,
		storageImageLayoutBinding,
		uniformBufferLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout), "failed to create descriptor set layout");

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VK_CHECK_RESULT(vkCreatePipelineLayout(device.getDevice(), &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout), "failed to create pipeline layout");
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
	uint32_t temp = device.getRTProperties()->maxRayRecursionDepth;
	creationInfo.maxPipelineRayRecursionDepth = std::max(3U, device.getRTProperties()->maxRayRecursionDepth);
	creationInfo.layout = graphicsPipelineLayout;

	PFN_vkVoidFunction address = vkGetInstanceProcAddr(*device.getInstance(), "vkCreateRayTracingPipelinesKHR");
	//PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(address);
	VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device.getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &creationInfo, nullptr, &graphicsPipeline), "failed to create ray tracing pipeline");
	createShaderBindingTable(&bindingTable, creationInfo.groupCount);
	//bindingTable = createShaderBindingTable(device, &graphicsPipeline, creationInfo.groupCount);
	//bindingTableBuffer = RTBindingTableBuffer(device, &graphicsPipeline, creationInfo.groupCount); //creates the shader binding table
}

void Core::Pipeline::createShaderStages(std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount>* shaderStages) {
	//Ray Generation Shader
	(*shaderStages)[eRaygen].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eRaygen].pNext = nullptr; //TODO: Shader code
	(*shaderStages)[eRaygen].pName = "rgenMain"; //Start point for shader
	(*shaderStages)[eRaygen].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	(*shaderStages)[eRaygen].pNext = nullptr; //Pointer to shader code

	//Miss Shader
	(*shaderStages)[eMiss].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eMiss].pNext = nullptr;
	(*shaderStages)[eMiss].pName = "rmissMain"; //Start point for shader
	(*shaderStages)[eMiss].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	(*shaderStages)[eMiss].pNext = nullptr; //Pointer to shader code

	//Closest Hit Shader
	(*shaderStages)[eClosestHit].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eClosestHit].pNext = nullptr;
	(*shaderStages)[eClosestHit].pName = "chitMain";
	(*shaderStages)[eClosestHit].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	(*shaderStages)[eClosestHit].pNext = nullptr; //Pointer to shader code
}

void Core::Pipeline::createShaderGroups(std::vector<VkRayTracingShaderGroupCreateInfoKHR>* shader_groups) {
	{
		VkRayTracingShaderGroupCreateInfoKHR group;
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = eRaygen;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups->push_back(group);
	}

	{
		VkRayTracingShaderGroupCreateInfoKHR group;
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = eMiss;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups->push_back(group);
	}

	{
		VkRayTracingShaderGroupCreateInfoKHR group;
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = eClosestHit;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups->push_back(group);
	}
}

void Core::Pipeline::createShaderBindingTable(ShaderBindingTable* bindingTable, uint32_t groupCount) {
	const uint32_t groupHandleSize = device.getRTProperties()->shaderGroupHandleSize;
	const uint32_t groupHandleAlignment = device.getRTProperties()->shaderGroupHandleAlignment;
	//const uint32_t groupCount = groupCount; Provided by the Ray Tracing Pipeline
	size_t dataSize = groupHandleSize * groupCount;

	std::vector<uint8_t> shaderHandles(dataSize);
	const PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR =
		reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetInstanceProcAddr(*device.getInstance(), "vkGetRayTracingShaderGroupHandlesKHR"));
	VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(device.getDevice(), graphicsPipeline, 0, groupCount, dataSize, shaderHandles.data()), "failed to get shader group handles");

	const VkBufferUsageFlags usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	Buffer raygen{ device, groupHandleSize, usage, properties };
	Buffer miss{ device, groupHandleSize, usage, properties };
	Buffer hit{ device, groupHandleSize, usage, properties };

	raygen.map();
	miss.map();
	hit.map();

	memcpy(raygen.getMappedMemory(), shaderHandles.data(), groupHandleSize);
	memcpy(miss.getMappedMemory(), shaderHandles.data() + groupHandleSize, groupHandleSize);
	memcpy(hit.getMappedMemory(), shaderHandles.data() + 2 * groupHandleSize, groupHandleSize);

	ShaderBindingTable sbt{
		&raygen,
		&miss,
		&hit,
	};

	*bindingTable = sbt;
}