#include "be_vulkanInstance.hpp"
#include "be_errorHandler.hpp"
#include "be_vulkanApp.hpp"

#include <GLFW/glfw3.h>
#include <string>

namespace be{

VkInstance VulkanInstance::getInstance() const {
    return _Instance;
}

void VulkanInstance::vulkanDestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator
    ) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanInstance::cleanUp(){
    if(VulkanApp::_VulkanEnableValidationLayers){
        vulkanDestroyDebugUtilsMessengerEXT(_Instance, _DebugMessenger, nullptr);
        vkDestroyInstance(_Instance, nullptr);
    }
}

void VulkanInstance::init(){
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = vulkanGetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    std::vector<VkExtensionProperties> supportedExtensions = vulkanGetSupportedExtensions();

    std::vector<VkLayerProperties> availableLayers = vulkanGetAvailableLayers();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if(VulkanApp::_VulkanEnableValidationLayers){
        vulkanCheckValidationLayerSupport(availableLayers);
        fprintf(stdout, "Validation layers checked!\n");
        createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanApp::_VulkanValidationLayers.size());
        createInfo.ppEnabledLayerNames = VulkanApp::_VulkanValidationLayers.data();
        vulkanPopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &_Instance);
    ErrorHandler::vulkanError(__FILE__, __LINE__, result, "Failed to create the vulkan instance!\n");
}

std::vector<const char*> VulkanInstance::vulkanGetRequiredExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(VulkanApp::_VulkanEnableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<VkExtensionProperties> VulkanInstance::vulkanGetSupportedExtensions(){
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    return extensions;
}

std::vector<VkLayerProperties> VulkanInstance::vulkanGetAvailableLayers(){
    uint32_t layerCout = 0;
    vkEnumerateInstanceLayerProperties(&layerCout, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCout);
    vkEnumerateInstanceLayerProperties(&layerCout, availableLayers.data());
    return availableLayers;
}

void VulkanInstance::vulkanCheckValidationLayerSupport(const std::vector<VkLayerProperties>& availableLayers){
    for(const char* layerName : VulkanApp::_VulkanValidationLayers){
        bool layerFound = false;
        for(const auto& layerProperties : availableLayers){
            if(strcmp(layerName, layerProperties.layerName) == 0){
                layerFound = true;
                break;
            }
        }
        if(!layerFound){
            ErrorHandler::handle(__FILE__, __LINE__, ErrorCode::VULKAN_ERROR, "Not all the validation layers are available!\n");
        }
    }
}

void VulkanInstance::vulkanPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = VulkanInstance::vulkanDebugCallback;
    createInfo.pUserData = nullptr; // Optional
}

void VulkanInstance::vulkanSetupDebugMessenger(){
    if(!VulkanApp::_VulkanEnableValidationLayers){
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    vulkanPopulateDebugMessengerCreateInfo(createInfo);

    VkResult result = vulkanCreateDebugUtilsMessengerEXT(
                _Instance, &createInfo, nullptr, &_DebugMessenger
                        );
    ErrorHandler::vulkanError(__FILE__, __LINE__, result, "Failed to set up debug messenger!\n");
    fprintf(stdout, "Debug messenger set up properly!\n");
}

VkResult VulkanInstance::vulkanCreateDebugUtilsMessengerEXT(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkBool32 VulkanInstance::vulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity[[maybe_unused]],
        VkDebugUtilsMessageTypeFlagsEXT messageType[[maybe_unused]],
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData[[maybe_unused]]
    ){
    fprintf(stdout, "Validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

void VulkanInstance::vulkanCheckGlfwExtensions(
    const std::vector<VkExtensionProperties>& supportedExtensions, 
    const char** glfwExtensions, uint32_t nbGlfwExtensions){

    bool allFound = true;
    for(int index = 0; index < int(nbGlfwExtensions); index++){
        bool found = false;
        const char* glfwExtension = glfwExtensions[index];
        for(const auto& extension : supportedExtensions){
            if(strcmp(glfwExtension, extension.extensionName) == 0){
                found = true;
                break;
            }
        }
        if(!found){
            ErrorHandler::handle(__FILE__, __LINE__, 
                ErrorCode::GLFW_ERROR, 
                "GLFW extension: " + std::string(glfwExtension) + " is not supported!\n",
                ErrorLevel::WARNING
            );
        }
    }
    if(allFound){
        fprintf(stdout, "All glfw extensions are supported!\n");
    }
}

};