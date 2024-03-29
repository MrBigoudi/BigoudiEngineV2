#include "be_buffer.hpp"

#include "be_errorHandler.hpp"

#include <cstring>

namespace be{


/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg minUniformBufferOffsetAlignment)
 * @return VkResult of the buffer mapping call
*/
VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }
    return instanceSize;
}

Buffer::Buffer(
    VulkanAppPtr vulkanApp,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
        : _VulkanApp{vulkanApp},
        _InstanceCount{instanceCount},
        _InstanceSize{instanceSize},
        _UsageFlags{usageFlags},
        _MemoryPropertyFlags{memoryPropertyFlags} {

    _AlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    _BufferSize = _AlignmentSize * instanceCount;
    _VulkanApp->createBuffer(
        _BufferSize, 
        usageFlags, 
        memoryPropertyFlags, 
        _Buffer, 
        _Memory
    );
}

void Buffer::cleanUp(){
    unmap();
    vkDestroyBuffer(_VulkanApp->getDevice(), _Buffer, nullptr);
    vkFreeMemory(_VulkanApp->getDevice(), _Memory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 * @return VkResult of the buffer mapping call
 */
VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset){
    if(!_Buffer || !_Memory){
        ErrorHandler::handle(__FILE__, __LINE__, 
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Can't map a buffer before creation!\n"
        );
    }
    return vkMapMemory(_VulkanApp->getDevice(), _Memory, offset, size, 0, &_Mapped);
}

/**
 * Unmap a mapped memory range
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void Buffer::unmap() {
    if(_Mapped){
        vkUnmapMemory(_VulkanApp->getDevice(), _Memory);
        _Mapped = nullptr;
    }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 */
void Buffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset){
    if(!_Mapped){
        ErrorHandler::handle(__FILE__, __LINE__, 
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Can't copy to an unmapped buffer!\n"
        );
    }

    if(size == VK_WHOLE_SIZE){
        memcpy(_Mapped, data, _BufferSize);
    }else{
        char *memOffset = (char *)_Mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 * @note Only required for non-coherent memory
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 * @return VkResult of the flush call
 */
VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset){
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(_VulkanApp->getDevice(), 1, &mappedRange);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 * @note Only required for non-coherent memory
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 * @return VkResult of the invalidate call
 */
VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset){
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = _Memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(_VulkanApp->getDevice(), 1, &mappedRange);
}

/**
 * Create a buffer info descriptor
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset){
    return VkDescriptorBufferInfo{
        _Buffer,
        offset,
        size,
    };
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 */
void Buffer::writeToIndex(void *data, int index){
    writeToBuffer(data, _InstanceSize, index * _AlignmentSize);
}

/**
 * Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 * @param index Used in offset calculation
 */
VkResult Buffer::flushIndex(int index){ 
    return flush(_AlignmentSize, index * _AlignmentSize); 
}

/**
 * Create a buffer info descriptor
 * @param index Specifies the region given by index * alignmentSize
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index){
    return descriptorInfo(_AlignmentSize, index * _AlignmentSize);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 * @note Only required for non-coherent memory
 * @param index Specifies the region to invalidate: index * alignmentSize
 * @return VkResult of the invalidate call
 */
VkResult Buffer::invalidateIndex(int index){
    return invalidate(_AlignmentSize, index * _AlignmentSize);
}



VkCommandBuffer Buffer::beginSingleTimeCommands(VulkanAppPtr vulkanApp){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanApp->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vulkanApp->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Buffer::endSingleTimeCommands(VulkanAppPtr vulkanApp, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanApp->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanApp->getGraphicsQueue());

    vkFreeCommandBuffers(vulkanApp->getDevice(), vulkanApp->getCommandPool(), 1, &commandBuffer);
}

void Buffer::copyBuffer(VulkanAppPtr vulkanApp, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkanApp);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(vulkanApp, commandBuffer);
}

void Buffer::copyBufferToImage(VulkanAppPtr vulkanApp, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height){
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vulkanApp);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(vulkanApp, commandBuffer);
}


};