#include "be_texture.hpp"

#define STBI_FAILURE_USERMSG
#include <stb_image.h>

namespace be{

void TextureImage::createImageView(){
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = _Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = _Format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(_VulkanApp->getDevice(), &viewInfo, nullptr, &_ImageView);
    ErrorHandler::vulkanError(
        __FILE__, __LINE__,
        result, 
        "Failed to create texture image view!\n"
    );



}

void TextureImage::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout){
    VkCommandBuffer commandBuffer = Buffer::beginSingleTimeCommands(_VulkanApp);
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = _Image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::VULKAN_ERROR, 
            "Unsupported layout transition!\n"
        );
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    Buffer::endSingleTimeCommands(_VulkanApp, commandBuffer);
}



TextureImage TextureImage::load(VulkanAppPtr vulkanApp, ImagePtr image){
    if(!image){
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a texture from a null image!\n"
        );
    }

    TextureImage newImage{};
    newImage._VulkanApp = vulkanApp;
    newImage._TexWidth = image->getWidth();
    newImage._TexHeight = image->getHeight();
    newImage._TexChannels = 4;

    VkDeviceSize imageSize = newImage._TexWidth * newImage._TexHeight * newImage._TexChannels;

    Buffer buffer = Buffer(
        vulkanApp, 
        imageSize,
        1, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    buffer.map();
    buffer.writeToBuffer(Image::pixelsToVectorOfBytes(image->getPixels()).data());
    buffer.unmap();

    newImage.init(
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    newImage.transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    Buffer::copyBufferToImage(vulkanApp, buffer.getBuffer(), newImage._Image, static_cast<uint32_t>(newImage._TexWidth), static_cast<uint32_t>(newImage._TexHeight));
    newImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    buffer.cleanUp();

    newImage.createImageView();
    newImage.createTextureSampler();

    return newImage;
}


TextureImage TextureImage::load(VulkanAppPtr vulkanApp, const std::string& path){
    TextureImage newImage{};
    newImage._VulkanApp = vulkanApp;

    stbi_uc* pixels = stbi_load(path.c_str(), &newImage._TexWidth, &newImage._TexHeight, &newImage._TexChannels, 0);
    VkDeviceSize imageSize = newImage._TexWidth * newImage._TexHeight * newImage._TexChannels;
    fprintf(stdout, "channels: %d\n", newImage._TexChannels);

    if(!pixels) {
        ErrorHandler::handle(
            __FILE__, __LINE__, 
            ErrorCode::STB_IMAGE_ERROR,    
            "Failed to load texture image: `" + path + "'\n\t" + stbi_failure_reason() + "\n"
        );
    }

    Buffer buffer = Buffer(
        vulkanApp, 
        imageSize,
        1, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    buffer.map();
    buffer.writeToBuffer(pixels);
    buffer.unmap();

    stbi_image_free(pixels);

    newImage.init(
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    newImage.transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    Buffer::copyBufferToImage(vulkanApp, buffer.getBuffer(), newImage._Image, static_cast<uint32_t>(newImage._TexWidth), static_cast<uint32_t>(newImage._TexHeight));
    newImage.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    buffer.cleanUp();

    newImage.createImageView();
    newImage.createTextureSampler();

    return newImage;
}

void TextureImage::cleanUp(){
    vkDestroySampler(_VulkanApp->getDevice(), _ImageSampler, nullptr);
    vkDestroyImageView(_VulkanApp->getDevice(), _ImageView, nullptr);
    vkDestroyImage(_VulkanApp->getDevice(), _Image, nullptr);
    vkFreeMemory(_VulkanApp->getDevice(), _ImageMemory, nullptr);
}

void TextureImage::init(VkFormat format, 
    VkImageTiling tiling, VkImageUsageFlags usage, 
    VkMemoryPropertyFlags properties
    ){

    _Format = format;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(_TexWidth);
    imageInfo.extent.height = static_cast<uint32_t>(_TexHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional
    
    VkResult result = vkCreateImage(_VulkanApp->getDevice(), &imageInfo, nullptr, &_Image);
    ErrorHandler::vulkanError(__FILE__, __LINE__, result, "Failed to create the texture image!\n");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_VulkanApp->getDevice(), _Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _VulkanApp->findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(_VulkanApp->getDevice(), &allocInfo, nullptr, &_ImageMemory);
    ErrorHandler::vulkanError(
        __FILE__, __LINE__, result,"Failed to allocate image memory!\n"
    );

    vkBindImageMemory(_VulkanApp->getDevice(), _Image, _ImageMemory, 0);
}

void TextureImage::createTextureSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_VulkanApp->getPhysicalDevice(), &properties);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(_VulkanApp->getDevice(), &samplerInfo, nullptr, &_ImageSampler);
    ErrorHandler::vulkanError(
        __FILE__, __LINE__, result,
        "Failed to create texture sampler!\n"
    );
}

VkDescriptorImageInfo TextureImage::getDescriptorInfo() const {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = _ImageView;
    imageInfo.sampler = _ImageSampler;
    return imageInfo;
}


void TextureUboContainer::update(uint32_t frameIndex){
    checkFrameIndex(frameIndex);
    _Ubos[frameIndex]->writeToBuffer(&_UboData);
}

void TextureUboContainer::init(uint32_t size, VulkanAppPtr vulkanApp){
    UboContainer::init(size, vulkanApp);
    _UboData = TextureUboData{};
    for(uint32_t i = 0; i<_Size; i++){
        _Ubos[i] = be::BufferPtr(new be::Buffer(
            vulkanApp, 
            sizeof(TextureUboData),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));
        _Ubos[i]->map();
    }
}

}