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
#include <chrono>

using TimePointType = decltype(std::chrono::high_resolution_clock::now());

struct SwapChainSupportDetail{
    VkSurfaceCapabilitiesKHR Capbilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
    }
};

class Rovski {
public:
    Rovski();
    virtual ~Rovski();
    void Run();
    bool Init(uint32_t windowWidth, uint32_t windowHeight, uint32_t maxFrameInFlight = 2);
    bool Clean();
    void OnFrameBufferSized();
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
    bool CreateFrameBuffer();
    bool CreateCommandPool();
    bool CreateCommandBuffer();
    bool CreateSyncObjects();
    void DrawFrame();
    void RecreateSwapChain();
    void CleanUpSwapChain();
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, bool needTransfer);
    bool CopyBuffer(VkBuffer &dst, VkBuffer &src, VkDeviceSize size);
    bool CreateDescriptorLayout();
    bool CreateUniformBuffers();
    void UpdateUniformBuffer(uint32_t imageIndex);
    void UpdateTime();

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
    VkQueue vkTransferQueue;
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkSwapchainKHR vkSwapChain;
    std::vector<VkImage> vkSwapChainImages;
    VkFormat vkSwapChainFormat;
    VkExtent2D vkSwapChainExtent;
    std::vector<VkImageView> vkSwapChainImageViews;
    VkRenderPass vkRenderPass;
    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkPipelineLayout vkPipelineLayout;
    VkPipeline vkGraphicsPipeline;
    std::vector<VkFramebuffer> vkSwapChainFrameBuffers;
    VkCommandPool vkCommandPool;
    std::vector<VkCommandBuffer> vkCommandBuffer;
    std::vector<VkSemaphore> vkImageAvailableSemaphore;
    std::vector<VkSemaphore> vkRenderFinishSemaphore;
    std::vector<VkFence> vkInFlightFences;
    uint32_t maxFrameInFlight;
    uint64_t currentFrame = 0;
    std::vector<VkFence> vkImagesInFlight;
    bool frameBufferResized = false;
    VkBuffer vkVertexBuffer;
    VkDeviceMemory vkVertextBufferMemory;
    VkBuffer vkIndexBuffer;
    VkDeviceMemory vkIndextBufferMemory;
    VkCommandPool vkTransferCommandPool;
    std::vector<VkDeviceMemory> vkUniformBuffersMemory;
    std::vector<VkBuffer> vkUniformBuffers;
    TimePointType startTime;
    TimePointType currentTime;
    double currentTimeFromStart = 0;
    double preFrameTimeFromStart = 0;
    double deltaTime = 0;
    
    static const std::vector<const char*> deviceExtensions;
    static const std::vector<const char*> validationLayers;

#if NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif
};

#endif /* Rovski_hpp */
