#include "Pipeline.h"
#include <fstream>

/*
 * Ray Tracing Pipeline has to return depths and normals for each pixel. 
 * This is required for the denoising process. 
 * The denoise process uses the depth and normal information to better understand the scene geometry and improve the quality of the denoised image.
 */

/*Core::Pipeline::Pipeline(Device& device, Scene& scene, SwapChain& swapChain) : device(device), scene(scene), swapChain(swapChain) {
	createRayTracingPipelineLayout();
	createStorageImages();
	createDescriptorSets();
	createRayTracingPipeline();
}

Core::Pipeline::~Pipeline() {
	vkDestroyPipeline(device.getDevice(), graphicsPipeline, nullptr); //Destorys the rendering pipeline
	vkDestroyPipelineLayout(device.getDevice(), graphicsPipelineLayout, nullptr); //Destroys the rendering pipeline layout
}

void Core::Pipeline::createDescriptorSets() {

	const uint32_t maxFrames = swapChain.MAX_FRAMES_IN_FLIGHT;

	std::vector<VkDescriptorPoolSize> poolSizes = {
	{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, static_cast<uint32_t>(swapChain.MAX_FRAMES_IN_FLIGHT) },
	{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(swapChain.MAX_FRAMES_IN_FLIGHT) },
	{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(swapChain.MAX_FRAMES_IN_FLIGHT) },
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

	descriptorSet.resize(swapChain.MAX_FRAMES_IN_FLIGHT);

	for (auto i = 0; i < swapChain.MAX_FRAMES_IN_FLIGHT; i++) {
		vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &descriptorSet[i]);

		VkWriteDescriptorSetAccelerationStructureKHR accelInfo = {};
		accelInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accelInfo.accelerationStructureCount = 1;
		accelInfo.pAccelerationStructures = &scene.getTlas().handle;
	
		//link acceleration structure to descriptor set
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet[i];
		descriptorWrite.dstBinding = 0; //will have index 0 in shader (binding = 0)
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptorWrite.pNext = &accelInfo;

		VkDescriptorImageInfo outputImageInfo = {};
		outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		outputImageInfo.imageView = storageImages[i].imageView;

		VkWriteDescriptorSet outputImageWrite = {};
		outputImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		outputImageWrite.dstSet = descriptorSet[i];
		outputImageWrite.dstBinding = 1; //will have index 1 in shader (binding = 1)
		outputImageWrite.descriptorCount = 1;
		outputImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		outputImageWrite.pImageInfo = &outputImageInfo;

		VkDescriptorBufferInfo uniformBufferInfo = uniformBuffer.descriptorInfo();

		VkWriteDescriptorSet uniformBufferWrite = {};
		uniformBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferWrite.dstSet = descriptorSet[i];
		uniformBufferWrite.dstBinding = 2; //will have index 2 in shader (binding = 2)
		uniformBufferWrite.descriptorCount = 1;
		uniformBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferWrite.pBufferInfo = &uniformBufferInfo; //Pointer to uniform buffer info

		std::vector<VkWriteDescriptorSet> writeDescriptors = {
			descriptorWrite,
			outputImageWrite,
			uniformBufferWrite
		};

		vkUpdateDescriptorSets(device.getDevice(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
	}

	/*VK_CHECK_RESULT(vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &descriptorSet), "failed to allocate memory for descriptor sets");

	VkDescriptorImageInfo outputImageInfo = {};
	outputImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	outputImageInfo.imageView = swapChain.getImageView(0);

	VkWriteDescriptorSet outputImageWrite = {};
	outputImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	outputImageWrite.dstSet = descriptorSet;
	outputImageWrite.dstBinding = 1; //will have index 1 in shader (binding = 1)
	outputImageWrite.descriptorCount = 1;
	outputImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	//outputImageInfo.imageView = outputImageView;
	outputImageWrite.pImageInfo = &outputImageInfo;

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
	std::vector<char> raygenCode = readFile("shaders/raygen.rgen.spv");
	std::vector<char> missCode = readFile("shaders/miss.rmiss.spv");
	std::vector<char> closesthitCode = readFile("shaders/closesthit.rchit.spv");
	
	std::vector<char> code = readFile("shaders/raytracing.slang.spv");

	VkShaderModuleCreateInfo shaderCode {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	stages[eRaygen].pNext = &shaderCode;
	stages[eRaygen].pName = "rgenMain";
	stages[eRaygen].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	stages[eMiss].pNext = &shaderCode;
	stages[eMiss].pName = "rmissMain";
	stages[eMiss].stage = VK_SHADER_STAGE_MISS_BIT_KHR;

	stages[eClosestHit].pNext = &shaderCode;
	stages[eClosestHit].pName = "rchitMain";
	stages[eClosestHit].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	createShaderGroups(&shader_groups);

	std::cout << "Stages size: " << stages.size() << std::endl;
	std::cout << "Group size: " << shader_groups.size() << std::endl;

	//The error seems to be caused by the shader stages

	VkRayTracingPipelineCreateInfoKHR creationInfo = {};
	creationInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	creationInfo.stageCount = static_cast<uint32_t>(stages.size());
	creationInfo.pStages = stages.data();
	creationInfo.groupCount = static_cast<uint32_t>(shader_groups.size());
	creationInfo.pGroups = shader_groups.data();
	creationInfo.maxPipelineRayRecursionDepth = std::max(3U, device.getRTProperties()->maxRayRecursionDepth);
	creationInfo.layout = graphicsPipelineLayout;

	VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device.getDevice(), {}, {}, 1, &creationInfo, nullptr, &graphicsPipeline), "failed to create ray tracing pipeline");
	createShaderBindingTable(&bindingTable, creationInfo.groupCount);
}

void Core::Pipeline::createShaderStages(std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount>* shaderStages) {
	std::vector<char> raygenCode = readFile("shaders/raygen.rgen.spv");
	std::vector<char> missCode = readFile("shaders/miss.rmiss.spv");
	std::vector<char> closesthitCode = readFile("shaders/closesthit.rchit.spv");

	//Create shader modules
	createShaderModule(raygenCode, &raygenShaderModule);
	createShaderModule(missCode, &missShaderModule);
	createShaderModule(closesthitCode, &closestHitShaderModule);

	VkShaderModuleCreateInfo raygen{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = raygenCode.size(),
		.pCode = reinterpret_cast<const uint32_t*>(raygenCode.data())
	};
	VkShaderModuleCreateInfo miss{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = missCode.size(),
		.pCode = reinterpret_cast<const uint32_t*>(missCode.data())
	};
	VkShaderModuleCreateInfo closesthit{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = closesthitCode.size(),
		.pCode = reinterpret_cast<const uint32_t*>(closesthitCode.data())
	};

	VkShaderModuleCreateInfo test{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = 0,
		.pCode = nullptr
	};

	//Ray Generation Shader
	(*shaderStages)[eRaygen].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eRaygen].pNext = &raygenShaderModule;
	(*shaderStages)[eRaygen].pName = "main";
	(*shaderStages)[eRaygen].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	(*shaderStages)[eRaygen].flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;


	//Miss Shader
	(*shaderStages)[eMiss].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eMiss].pNext = &missShaderModule;
	(*shaderStages)[eMiss].pName = "main";
	(*shaderStages)[eMiss].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	(*shaderStages)[eMiss].flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;

	//Closest Hit Shader
	(*shaderStages)[eClosestHit].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	(*shaderStages)[eClosestHit].pNext = &closestHitShaderModule;
	(*shaderStages)[eClosestHit].pName = "main";
	(*shaderStages)[eClosestHit].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	(*shaderStages)[eClosestHit].flags = VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT;
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

void Core::Pipeline::createStorageImages() {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	//imageInfo.format = swapChain.getSwapChainImageFormat();
	imageInfo.extent.height = swapChain.getSwapChainExtent().height;
	imageInfo.extent.width = swapChain.getSwapChainExtent().width;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	storageImages.resize(swapChain.MAX_FRAMES_IN_FLIGHT);

	for(auto i = 0; i< swapChain.MAX_FRAMES_IN_FLIGHT; i++) {
		device.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			storageImages[i].image,
			storageImages[i].imageMemory);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		//viewInfo.format = swapChain.getSwapChainImageFormat();
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.image = storageImages[i].image;

		VK_CHECK_RESULT(vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &storageImages[i].imageView), "failed to create texture image view!");
		//storageImages.push_back(StorageImage(device, imageInfo, swapChain.getSwapChainExtent()));
	}
}

std::vector<char> Core::Pipeline::readFile(const std::string& filename) {
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };

	if(!file.is_open()) throw std::runtime_error("failed to open file: " + filename);

	size_t size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), size);
	file.close();
	return buffer;
}

void Core::Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VK_CHECK_RESULT(vkCreateShaderModule(device.getDevice(), &info, nullptr, shaderModule), "failed to create shader module");
}*/