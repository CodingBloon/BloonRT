#include "Buffer.h"
#include <cassert>

/*Core::Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize minOffsetAlignment) : bufferSize(size), device(device) {
	device->createBuffer(size, usage, properties, &buffer, &memory);
	gpuAddress = device->getBufferDeviceAddress(buffer);
}

Core::Buffer::~Buffer() {
	unmap();
	vkDestroyBuffer(*device->getDevice(), buffer, nullptr);
	vkFreeMemory(*device->getDevice(), memory, nullptr);
}

void Core::Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
	assert(mapped && "Cannot write to unmapped buffer memory.");
	
	if (size == VK_WHOLE_SIZE) {
		memcpy(mapped, data, bufferSize);
	}
	else {
		char* memOffset = (char*)mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}

}

VkDescriptorBufferInfo* Core::Buffer::getDescriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
	return new VkDescriptorBufferInfo{
		buffer,
		offset,
		size
	};
}

VkResult Core::Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkFlushMappedMemoryRanges(*device->getDevice(), 1, &mappedRange);
}

VkResult Core::Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkInvalidateMappedMemoryRanges(*device->getDevice(), 1, &mappedRange);
}

void Core::Buffer::map() {
	assert(mapped && memory && "Cannot map buffer memory: Memory is already mapped or not allocated.");
	VK_CHECK_RESULT(vkMapMemory(*device->getDevice(), memory, 0, bufferSize, 0, &mapped), "failed to map memory");
}

void Core::Buffer::unmap() {
	if (!mapped)
		return;
	vkUnmapMemory(*device->getDevice(), memory);
	mapped = nullptr;
}*/

VkDeviceSize Core::Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

Core::Buffer::Buffer(
    Device& device,
    VkDeviceSize size,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
    : lveDevice{ device },
    instanceSize{ instanceSize },
    usageFlags{ usageFlags },
	bufferSize{ size },
    memoryPropertyFlags{ memoryPropertyFlags } {
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, &buffer, &memory);
}

Core::Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(lveDevice.getDevice(), buffer, nullptr);
    vkFreeMemory(lveDevice.getDevice(), memory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult Core::Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    assert(buffer && memory && "Called map on buffer before create");
    return vkMapMemory(lveDevice.getDevice(), memory, offset, size, 0, &mapped);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void Core::Buffer::unmap() {
    if (mapped) {
        vkUnmapMemory(lveDevice.getDevice(), memory);
        mapped = nullptr;
    }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void Core::Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    assert(mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, bufferSize);
    }
    else {
        char* memOffset = (char*)mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult Core::Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(lveDevice.getDevice(), 1, &mappedRange);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult Core::Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(lveDevice.getDevice(), 1, &mappedRange);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo Core::Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}