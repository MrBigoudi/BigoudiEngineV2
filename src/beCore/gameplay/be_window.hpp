#pragma once

#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <cstdint>
#include <string>

#include "be_errorHandler.hpp"

namespace be{

class Window;
using WindowPtr = std::shared_ptr<Window>;

enum WindowInputMode{
    INPUT_STICKY_KEYS = GLFW_STICKY_KEYS,
    INPUT_LOCK_KEY_MODS = GLFW_LOCK_KEY_MODS,
};

class Window{

    private:
        uint32_t _Width = 0;
        uint32_t _Height = 0;
        std::string _Title = "";
        bool _FrameBufferResized = false;

        GLFWwindow* _Window = nullptr;

    public:
        Window(uint32_t width, uint32_t height, const std::string& title)
            : _Width{width}, _Height{height}, _Title{title}{}

        void cleanUp(){
            glfwDestroyWindow(_Window);
            glfwTerminate();
        }

        bool shouldClose() const {
            return glfwWindowShouldClose(_Window);
        }

        void init(){
            if(!glfwInit()){
                ErrorHandler::glfwError(__FILE__, __LINE__, "Failed to init GLFW!\n");
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            _Window = glfwCreateWindow(
                _Width, _Height, 
                _Title.c_str(), 
                nullptr, nullptr
            );

            if(_Window == nullptr){
                ErrorHandler::glfwError(__FILE__, __LINE__, "Failed to init GLFW window!\n");
            }

            // center the window
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            int screenWidth = mode->width;
            int screenHeight = mode->height;
            int windowXPos = (screenWidth - _Width) / 2;
            int windowYPos = (screenHeight - _Height) / 2;
            glfwSetWindowPos(_Window, windowXPos, windowYPos);

            glfwSetWindowUserPointer(_Window, this);
            glfwSetWindowSizeCallback(_Window, frameBufferResizeCallback);
            glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // glfwSetInputMode(_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            glfwMakeContextCurrent(_Window);
        }

        VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR& surface){
            return glfwCreateWindowSurface(instance, _Window, nullptr, &surface);
        }

        VkExtent2D getExtent(const VkSurfaceCapabilitiesKHR& capabilities){
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                int width, height;
                glfwGetFramebufferSize(_Window, &width, &height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                return actualExtent;
            }
        }

        bool wasWindowResized(){
            return _FrameBufferResized;
        }

        void resetWindowResizedFlag(){
            _FrameBufferResized = false;
        }

        GLFWwindow* getWindow() const {
            return _Window;
        }

        void setInputMode(WindowInputMode mode, bool value){
            glfwSetInputMode(getWindow(), mode, value ? GLFW_TRUE : GLFW_FALSE);
        }

        void setKeyCallback(GLFWkeyfun keyCallback){
            glfwSetKeyCallback(getWindow(), keyCallback);
        }

        void setMouseCallback(GLFWcursorposfun mouseCallback){
            glfwSetCursorPosCallback(getWindow(), mouseCallback);
        }

    private:
        static void frameBufferResizeCallback(GLFWwindow* window, int width, int height){
            auto newWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            newWindow->_FrameBufferResized = true;
            newWindow->_Width = static_cast<uint32_t>(width);
            newWindow->_Height = static_cast<uint32_t>(height);
        }

};

};