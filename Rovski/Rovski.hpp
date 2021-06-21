//
//  Rovski.hpp
//  Rovski
//
//  Created by 罗斌 on 2021/6/12.
//

#ifndef Rovski_hpp
#define Rovski_hpp

#include <stdio.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>
#include <string>

struct SwapChainSupportDetail{
    VkSurfaceCapabilitiesKHR Capbilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Rovski {
public:
    Rovski();
    virtual ~Rovski();
    void Run();
    bool Init(uint32_t windowWidth, uint32_t windowHeight);
    bool Clean();
    static VKAPI_ATTR VkBool32 VKAPI_CALL VkApiCallDebugCallBack(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *CallBackData, void* userData);
private:
    bool InitWindow();
    bool InitVulkan();
    bool CreateVkInstance();
    bool CheckoutValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    bool SetDebugMessage();
    void DestroyDebugMessager();
    void FillCreateDebugInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo);
    bool PickPhysicCard();
    int RateDevice(VkPhysicalDevice device);
    bool CreateLogicalDevice();
    bool CreateSurface();
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
    bool CheckDeviceExtSupport(VkPhysicalDevice);
    SwapChainSupportDetail QuerrySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &avialablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilites);
    bool CreateSwapChain();
    bool CreateImageViews();
    bool CreateGraphicsPipeline();
    bool CreateShaderModule(const std::vector<char> &code, VkShaderModule &shaderModule);
    bool CreateRenderPass();

    
    VkInstance vkInstance;
    GLFWwindow* window;
    uint32_t windowWidth;
    uint32_t windowHeight;
    VkSurfaceKHR vkSurface;
    VkDebugUtilsMessengerEXT vkDebugMessager;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice;
    VkQueue vkGraphicsQueue;
    VkQueue vkPresentQueue;
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkSwapchainKHR vkSwapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat vkSwapChainFormat;
    VkExtent2D vkSwapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass vkRenderPass;
    VkPipelineLayout vkPipelineLayout;
    
    static const std::vector<const char*> deviceExtensions;
    static const std::vector<const char*> validationLayers;

#if NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif
};

#endif /* Rovski_hpp */
