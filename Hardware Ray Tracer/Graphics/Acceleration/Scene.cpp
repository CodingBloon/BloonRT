#include "Scene.h"
#include <span>

/*Core::Scene::Scene(Device& device, std::vector<Mesh> meshes) : device(device), meshes(meshes) {
	createBottomLevelAs();
	createTopLevelAs();
}

Core::Scene::~Scene() {
	vkFreeMemory(device.getDevice(), m_tlasAccel.memory, nullptr);
	vkDestroyBuffer(device.getDevice(), m_tlasAccel.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(device.getDevice(), m_tlasAccel.handle, nullptr);

	for(uint32_t i = 0; i < m_blasAccel.size(); i++) {
		vkFreeMemory(device.getDevice(), m_blasAccel[i].memory, nullptr);
		vkDestroyBuffer(device.getDevice(), m_blasAccel[i].buffer, nullptr);
		vkDestroyAccelerationStructureKHR(device.getDevice(), m_blasAccel[i].handle, nullptr);
	}
}

void Core::Scene::primitiveToGeometry(Mesh& mesh, VkAccelerationStructureGeometryKHR& geometry, VkAccelerationStructureBuildRangeInfoKHR& range) {

	auto triangleCount = static_cast<uint32_t>(mesh.indices.size() / 3);

	VkAccelerationStructureGeometryTrianglesDataKHR triangles{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
		.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,  // vec3 vertex position data
		.vertexData = { .deviceAddress = mesh.vertexBuffer.getAddress() },
		.vertexStride = sizeof(Vertex),
		.maxVertex = static_cast<uint32_t>(mesh.vertices.size() - 1),
		.indexType = VK_INDEX_TYPE_UINT32,  // Index type (VK_INDEX_TYPE_UINT16 or VK_INDEX_TYPE_UINT32)
		.indexData = {.deviceAddress = mesh.indexBuffer.getAddress() },
	};

	geometry = VkAccelerationStructureGeometryKHR{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		.geometry = {.triangles = triangles },
		.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR | VK_GEOMETRY_OPAQUE_BIT_KHR

	};

	range = VkAccelerationStructureBuildRangeInfoKHR{
		.primitiveCount = triangleCount,
	};
}

void Core::Scene::createAccelerationStructure(VkAccelerationStructureTypeKHR asType, 
	AccelerationStructure& AS, 
	VkAccelerationStructureGeometryKHR& geometry, 
	VkAccelerationStructureBuildRangeInfoKHR& range, 
	VkBuildAccelerationStructureFlagsKHR flags) {

	VkAccelerationStructureBuildGeometryInfoKHR asBuildInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = asType,
		.flags = flags,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.geometryCount = 1,
		.pGeometries = &geometry
	};

	std::vector<uint32_t> maxPrimitiveCounts(1);
	maxPrimitiveCounts[0] = range.primitiveCount;


	VkAccelerationStructureBuildSizesInfoKHR asBuildSize{};
	asBuildSize.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	
	vkGetAccelerationStructureBuildSizesKHR(device.getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asBuildInfo, maxPrimitiveCounts.data(), &asBuildSize);

	Buffer scratchBuffer{
		device,
		asBuildSize.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	VkAccelerationStructureCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	createInfo.size = asBuildSize.accelerationStructureSize;
	createInfo.type = asType;

	createAcceleration(AS, createInfo);

	VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
	asBuildInfo.dstAccelerationStructure = AS.handle;
	asBuildInfo.scratchData.deviceAddress = scratchBuffer.getAddress();

	VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &range;
	vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &asBuildInfo, &pBuildRangeInfo);

	device.endSingleTimeCommands(commandBuffer);
}

void Core::Scene::createBottomLevelAs() {
	m_blasAccel.resize(meshes.size());

	for (uint32_t i = 0; i < meshes.size(); i++) {
		VkAccelerationStructureGeometryKHR asGeometry{};
		VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};

		primitiveToGeometry(meshes[i], asGeometry, asBuildRangeInfo);
		createAccelerationStructure(
			VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			m_blasAccel[i],
			asGeometry,
			asBuildRangeInfo,
			VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR
		);
	}
}

void Core::Scene::createTopLevelAs() {
	VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f
	};

	std::vector<VkAccelerationStructureInstanceKHR> tlasInstances;
	tlasInstances.resize(m_blasAccel.size());
	for(uint32_t i = 0; i < m_blasAccel.size(); i++) {
		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = m_blasAccel[i].address;
		tlasInstances[i] = instance;
	}

	VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

	Buffer tlasInstancesBuffer{
		device,
		std::span<VkAccelerationStructureInstanceKHR const>(tlasInstances).size_bytes(),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	tlasInstancesBuffer.map();
	tlasInstancesBuffer.writeToBuffer(tlasInstances.data(), std::span<VkAccelerationStructureInstanceKHR const>(tlasInstances).size_bytes(), 0);
	device.endSingleTimeCommands(commandBuffer);


	VkAccelerationStructureGeometryKHR       asGeometry{};
	VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};

	VkAccelerationStructureGeometryInstancesDataKHR geomertyInstances{};
	geomertyInstances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geomertyInstances.data.deviceAddress = tlasInstancesBuffer.getAddress();

	uint32_t instanceCount = static_cast<uint32_t>(m_blasAccel.size());

	asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	asGeometry.geometry.instances = geomertyInstances;
	asBuildRangeInfo.primitiveCount = instanceCount;

	createAccelerationStructure(
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		m_tlasAccel,
		asGeometry,
		asBuildRangeInfo,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR
	);
}

void Core::Scene::createAcceleration(AccelerationStructure& structure, VkAccelerationStructureCreateInfoKHR info) {
	//create Buffer for acceleration Structure
	device.createBuffer(
		info.size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		&structure.buffer,
		&structure.memory
	);

	structure.address = device.getBufferDeviceAddress(structure.buffer);
	info.buffer = structure.buffer;
	vkCreateAccelerationStructureKHR(device.getDevice(), &info, nullptr, &structure.handle);

}*/