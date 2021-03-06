//
//  Rovski.cpp
//  Rovski
//
//  Created by 罗斌 on 2021/6/12.
//

#include "Rovski.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include "BaseStructs.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Vertex v = std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f});
std::vector<Vertex> Vertices = {
        std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f}),
        std::make_tuple(glm::vec3{0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{0.0f, 0.0f}),
        std::make_tuple(glm::vec3{0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec2{0.0f, 1.0f}),
        std::make_tuple(glm::vec3{-0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec2{1.0f, 1.0f}),

        std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f}),
        std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f}),
        std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f}),
        std::make_tuple(glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec2{1.0f, 0.0f}),
};




/*

        {{-0.5f, -0.5f, -0.5}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
};
 */

std::vector<uint16_t> Indexes = {
        0,1,2,2,3,0,
        4,5,6,6,7,4
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 prj;
};

const std::vector<const char*> Rovski::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> Rovski::deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static void FrameBufferResizeCallback(GLFWwindow* window, int height, int width){
    Rovski *rovski = reinterpret_cast<Rovski*>(glfwGetWindowUserPointer(window));
    if (rovski != nullptr) {
        rovski->OnFrameBufferSized();
    }
}

Rovski::Rovski(){
    
}

Rovski::~Rovski(){
    
}

void Rovski::OnFrameBufferSized() {
    frameBufferResized = true;
}

void Rovski::Run(){
    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        UpdateTime();
        glfwPollEvents();
        DrawFrame();
    }
    vkDeviceWaitIdle(vkDevice);
}

bool Rovski::Init(uint32_t windowWidth, uint32_t windowHeight, uint32_t maxFrameInFlight) {
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
    this->maxFrameInFlight = maxFrameInFlight;
    InitWindow();
    InitVulkan();
    startTime = std::chrono::high_resolution_clock::now();
    return true;
}

bool Rovski::Clean(){
    CleanUpSwapChain();
    vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, nullptr);
    vkDestroySampler(vkDevice, vkTextureSampler, nullptr);
    vkDestroyImageView(vkDevice, vkTextureImageView, nullptr);
    vkDestroyImage(vkDevice, vkTextureImage, nullptr);
    vkFreeMemory(vkDevice, vkTextureMemory, nullptr);
    vkDestroyBuffer(vkDevice, vkIndexBuffer, nullptr);
    vkFreeMemory(vkDevice, vkIndextBufferMemory, nullptr);
    vkDestroyBuffer(vkDevice, vkVertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vkVertextBufferMemory, nullptr);
    for (int i = 0; i < maxFrameInFlight; i++) {
        vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore[i], nullptr);
        vkDestroySemaphore(vkDevice, vkRenderFinishSemaphore[i], nullptr);
        vkDestroyFence(vkDevice, vkInFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
    vkDestroyCommandPool(vkDevice, vkTransferCommandPool, nullptr);
    vkDestroyDevice(vkDevice, nullptr);
    DestroyDebugMessager();
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    vkDestroyInstance(vkInstance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return true;
}

bool Rovski::InitWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(windowWidth, windowHeight, "Rovski", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, FrameBufferResizeCallback);
    glfwSetWindowUserPointer(window, this);
    return true;
}

bool Rovski::InitVulkan(){
    CreateVkInstance();
    CreateSurface();
    SetDebugMessage();
    if (!PickPhysicCard()){
        std::cout << "failed to find a suitable physic card." << std::endl;
        return false;
    }
    if (!CreateLogicalDevice()) {
        std::cout << "failed to create logical device" << std::endl;
        return false;
    }
    if (!CreateSwapChain()) {
        std::cout << "failed to create swap chain" << std::endl;
        return false;
    }
    if (!CreateImageViews()) {
        std::cout << "failed to create image view" << std::endl;
        return false;
    }
    if (!CreateRenderPass()) {
        std::cout << "failed to create render pass" << std::endl;
        return false;
    }
    if (!CreateDescriptorLayout()) {
        std::cout << "failed to create descriptor pipeline" << std::endl;
        return false;
    }
    if (!CreateGraphicsPipeline()) {
        std::cout << "failed to create graphics pipeline" << std::endl;
        return false;
    }
    if (!CreateFrameBuffer()) {
        std::cout << "failed to create frame buffers" << std::endl;
        return false;
    }
    if (!CreateCommandPool()) {
        std::cout << "failed to create command pool" << std::endl;
        return false;
    }
    if (!CreateTextureImage()) {
        std::cout << "failed to create texture image" << std::endl;
        return false;
    }
    if (!CreateTextureImageView()) {
        std::cout << "failed to create texture image view" << std::endl;
        return false;
    }
    if (!CreateTextureSampler()) {
        std::cout << "failed to create texture sampler" << std::endl;
        return false;
    }
    if (!CreateVertexBuffer()) {
        std::cout << "failed to create vertex buffer" << std::endl;
        return false;
    }
    if (!CreateIndexBuffer()) {
        std::cout << "failed to create vertex buffer" << std::endl;
        return false;
    }
    if (!CreateUniformBuffers()){
        std::cout << "failed to create uniform buffer" << std::endl;
        return false;
    }
    if (!CreateDescriptorPool()) {
        std::cout << "failed to create descriptor pool" << std::endl;
        return false;
    }
    if (!CreateDescriptorSet()) {
        std::cout << "failed to create descriptor set" << std::endl;
        return false;
    }
    if (!CreateCommandBuffer()) {
        std::cout << "failed to create command buffer" << std::endl;
        return false;
    }
    if (!CreateSyncObjects()) {
        std::cout << "failed to create semaphores" << std::endl;
        return false;
    }
    return true;
}

bool Rovski::CreateVkInstance(){
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Rovski";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Rovski";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo insCreateInfo{};
    insCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    insCreateInfo.pApplicationInfo = &appInfo;
    auto extRequired = GetRequiredExtensions();
    insCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extRequired.size());
    insCreateInfo.ppEnabledExtensionNames = extRequired.data();
    std::cout << "extCnt: " << extRequired.size() << std::endl;
    for (auto ext:extRequired){
        std::cout << ext << std::endl;
    }
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers && !CheckoutValidationLayerSupport()) {
        throw std::runtime_error("validation layer is not support");
    }
    if (enableValidationLayers) {
        insCreateInfo.ppEnabledLayerNames = validationLayers.data();
        insCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        FillCreateDebugInfo(&debugCreateInfo);
        insCreateInfo.pNext = &debugCreateInfo;
    } else {
        insCreateInfo.enabledLayerCount = 0;
        insCreateInfo.pNext = nullptr;
    }
    return vkCreateInstance(&insCreateInfo, nullptr, &vkInstance) == VK_SUCCESS;
}

bool Rovski::CheckoutValidationLayerSupport() {
    uint32_t layerCnt;
    vkEnumerateInstanceLayerProperties(&layerCnt, nullptr);
    std::vector<VkLayerProperties> properties(layerCnt);
    vkEnumerateInstanceLayerProperties(&layerCnt, properties.data());
    std::cout << "Valid validation layer count: " << layerCnt << std::endl;
    for (int i = 0; i < layerCnt; i++) {
        std::cout << properties[i].layerName << ": " << properties[i].description
            << std::endl;
    }
    for (auto targetLayer : validationLayers) {
        bool found = false;
        for (auto layerWeGot : properties) {
            if (strcpy(layerWeGot.layerName, targetLayer)){
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "Validation layer not found: " << targetLayer << std::endl;
            return false;
        }
    }
    return true;
}

std::vector<const char*> Rovski::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Rovski::VkApiCallDebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callBackData, void* userData){
    std::cout << "validation Layer: " << callBackData->pMessage << std::endl;
    return VK_FALSE;
}

bool Rovski::SetDebugMessage() {
    if (!enableValidationLayers) {
        return true;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    FillCreateDebugInfo(&createInfo);
    auto createFunc = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (createFunc != nullptr) {
        return createFunc(vkInstance, &createInfo, nullptr, &vkDebugMessager) == VK_SUCCESS;
    } else {
        return false;
    }
}

void Rovski::FillCreateDebugInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = VkApiCallDebugCallBack;
    createInfo->pUserData = nullptr;
}

void Rovski::DestroyDebugMessager(){
    if (enableValidationLayers) {
        auto desFunc = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
        if(desFunc) {
            desFunc(vkInstance, vkDebugMessager, nullptr);
        }
    }
}

bool Rovski::PickPhysicCard() {
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        return false;
    }
    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    std::multimap<int, VkPhysicalDevice>  candidates;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, devices.data());
    for (auto &device : devices) {
        int score = RateDevice(device);
        candidates.insert(std::make_pair(score, device));
    }
    if (candidates.rbegin()->first > 0) {
        vkPhysicalDevice = candidates.rbegin()->second;
        /*
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);
        std::cout << "=======================================" << std::endl;
        std::cout << deviceProperties.limits.maxVertexInputAttributes << ", "
        << deviceProperties.limits.maxVertexInputBindings << std::endl;
        std::cout << "=======================================" << std::endl;
         */
        return true;
    }
    return false;
}

int Rovski::RateDevice(VkPhysicalDevice device){
    int score = 0;
    VkPhysicalDeviceProperties devicePropoerties{};
    vkGetPhysicalDeviceProperties(device, &devicePropoerties);
    vkGetPhysicalDeviceFeatures(device, &vkDeviceFeatures);
    if (vkDeviceFeatures.samplerAnisotropy != VK_TRUE) {
        score -= 100000;
    }
    if (devicePropoerties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }
    if (vkDeviceFeatures.geometryShader) {
        score += 100;
    }
    
    QueueFamilyIndices indices = FindQueueFamilies(device);
    if (indices.isComplete()) {
        score += 100;
    } else {
        score -= 100000;
    }
    if (!CheckDeviceExtSupport(device)){
        score -= 100000;
    }
    SwapChainSupportDetail detail = QuerrySwapChainSupport(device);
    if (detail.Formats.empty() || detail.PresentModes.empty()){
        score -= 100000;
    }
    return score;
}

QueueFamilyIndices Rovski::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (auto queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);
        if (presentSupport){
            indices.presentFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool Rovski::CreateLogicalDevice(){
    QueueFamilyIndices queueFamily = FindQueueFamilies(vkPhysicalDevice);;
    if (!queueFamily.isComplete()){
        std::cout << "failed to get grappic queue family" << std::endl;
        return false;
    }
    
    std::set<uint32_t> uniqueQueueFamilies = {queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value(), queueFamily.transferFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (auto family : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueCreateInfo);
    }
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceCreateInfo.pEnabledFeatures = &vkDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        return false;
    }
    
    vkGetDeviceQueue(vkDevice, queueFamily.graphicsFamily.value(), 0, &vkGraphicsQueue);
    vkGetDeviceQueue(vkDevice, queueFamily.presentFamily.value(), 0, &vkPresentQueue);
    vkGetDeviceQueue(vkDevice, queueFamily.transferFamily.value(), 0, &vkTransferQueue);

    return true;
}

bool Rovski::CreateSurface(){
    return glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface) == VK_SUCCESS;
}

bool Rovski::CheckDeviceExtSupport(VkPhysicalDevice device){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> avialbleExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, avialbleExtensions.data());
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (auto avialbleExtension : avialbleExtensions){
        requiredExtensions.erase(avialbleExtension.extensionName);
    }
    return requiredExtensions.empty();
}

SwapChainSupportDetail Rovski::QuerrySwapChainSupport(VkPhysicalDevice device){
    SwapChainSupportDetail detail;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &detail.Capbilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);
    if (formatCount > 0) {
        detail.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, detail.Formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        detail.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, detail.PresentModes.data());
    }
    return detail;
}

VkSurfaceFormatKHR Rovski::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (auto &format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR Rovski::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &avialablePresentModes) {
    for (const auto& avialablePresentMode : avialablePresentModes) {
        if (avialablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return avialablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Rovski::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilites) {
    if (capabilites.currentExtent.width != UINT32_MAX) {
        return capabilites.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
        return actualExtent;
    }
}

bool Rovski::CreateSwapChain(){
    SwapChainSupportDetail swapChainSupportDetail = QuerrySwapChainSupport(vkPhysicalDevice);
    VkSurfaceFormatKHR format = ChooseSwapSurfaceFormat(swapChainSupportDetail.Formats);
    VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupportDetail.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupportDetail.Capbilities);
    
    uint32_t imageCount = swapChainSupportDetail.Capbilities.minImageCount + 1;
    if (swapChainSupportDetail.Capbilities.maxImageCount > 0 && imageCount > swapChainSupportDetail.Capbilities.maxImageCount) {
        imageCount = swapChainSupportDetail.Capbilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = vkSurface;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = format.format;
    swapChainCreateInfo.imageColorSpace = format.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = FindQueueFamilies(vkPhysicalDevice);
    if (indices.graphicsFamily != indices.presentFamily || indices.transferFamily != indices.graphicsFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndeices[] = {indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value()};
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndeices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
    }
    
    swapChainCreateInfo.preTransform = swapChainSupportDetail.Capbilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(vkDevice, &swapChainCreateInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
        return false;
    }
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &swapChainImageCount, nullptr);
    vkSwapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &swapChainImageCount, vkSwapChainImages.data());
    vkSwapChainFormat = format.format;
    vkSwapChainExtent = extent;
    return true;
}

bool Rovski::CreateImageViews(){
    vkSwapChainImageViews.resize(vkSwapChainImages.size());
    for (int i = 0; i < vkSwapChainImages.size(); i++) {
        vkSwapChainImageViews[i] = CreateImageView(vkSwapChainImages[i], vkSwapChainFormat);
        if(vkSwapChainImageViews[i] == VK_NULL_HANDLE) {
            return false;
        }
    }
    return true;
}

bool Rovski::CreateShaderModule(const std::vector<char> &code, VkShaderModule &shaderModule){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    return VK_SUCCESS == vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule);
}

bool ReadFile(const std::string &fileName, std::vector<char> &buffer) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    size_t fileSize = (size_t)file.tellg();
    buffer.resize(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return true;
}

bool Rovski::CreateGraphicsPipeline(){
    std::vector<char> vertShaderCode(0),fragShaderCode(0);
    if (!ReadFile("/Users/luobin/Rovski/Rovski/Shader/vert.spv", vertShaderCode)){
        return false;
    }
    if(!ReadFile("/Users/luobin/Rovski/Rovski/Shader/frag.spv", fragShaderCode)){
        return false;
    }
    VkShaderModule vertShaderModule, fragShaderModule;
    if (CreateShaderModule(vertShaderCode, vertShaderModule) != true) {
        return false;
    }
    if (CreateShaderModule(fragShaderCode, fragShaderModule) != true) {
        return false;
    }
    
    VkPipelineShaderStageCreateInfo vertCreateInfo{};
    vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.module = vertShaderModule;
    vertCreateInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragCreateInfo{};
    fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragCreateInfo.module = fragShaderModule;
    fragCreateInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertCreateInfo, fragCreateInfo};

    auto vertexBinding = Vertex::getBindingDescription();
    auto vertexAttribute = Vertex::getVertexInputAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBinding;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribute.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttribute.data();
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    VkViewport viewPort{};
    viewPort.x = 0.0f;
    viewPort.y = 0.0f;
    viewPort.height = vkSwapChainExtent.height;
    viewPort.width = vkSwapChainExtent.width;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = vkSwapChainExtent;
    
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewPort;
    
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.0f;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    
    if(vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
        return false;
    }
    
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = vkPipelineLayout;
    pipelineCreateInfo.renderPass = vkRenderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if(vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkGraphicsPipeline) != VK_SUCCESS){
        return false;
    }
    
    
    vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
    
    return true;
}

bool Rovski::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vkSwapChainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkRenderPassCreateInfo renderPassCreateInfo {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &vkRenderPass) != VK_SUCCESS){
        return false;
    }
    
    return true;
}

bool Rovski::CreateFrameBuffer() {
    vkSwapChainFrameBuffers.resize(vkSwapChainImageViews.size());
    
    for (int i = 0; i < vkSwapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            vkSwapChainImageViews[i]
        };
        VkFramebufferCreateInfo frameBufferCreateInfo{};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = vkRenderPass;
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.pAttachments = attachments;
        frameBufferCreateInfo.width = vkSwapChainExtent.width;
        frameBufferCreateInfo.height = vkSwapChainExtent.height;
        frameBufferCreateInfo.layers = 1;
        if (vkCreateFramebuffer(vkDevice, &frameBufferCreateInfo, nullptr, vkSwapChainFrameBuffers.data() + i) != VK_SUCCESS) {
            return false;
        }
    }
    
    return true;
}

bool Rovski::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(vkPhysicalDevice);
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolCreateInfo.flags = 0;
    
    if (vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &vkCommandPool) != VK_SUCCESS){
        return false;
    }
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    return vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &vkTransferCommandPool) == VK_SUCCESS;
}

bool Rovski::CreateCommandBuffer() {
    vkCommandBuffer.resize(vkSwapChainFrameBuffers.size());
    VkCommandBufferAllocateInfo commandBufferAllocInfo{};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(vkCommandBuffer.size());
    commandBufferAllocInfo.commandPool = vkCommandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    if(vkAllocateCommandBuffers(vkDevice, &commandBufferAllocInfo, vkCommandBuffer.data()) != VK_SUCCESS) {
        return false;
    }
    
    for (int i = 0; i < vkCommandBuffer.size(); i++) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(vkCommandBuffer[i], &commandBufferBeginInfo) != VK_SUCCESS) {
            return false;
        }
        
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vkRenderPass;
        renderPassBeginInfo.framebuffer = vkSwapChainFrameBuffers[i];
        renderPassBeginInfo.renderArea.offset = {0,0};
        renderPassBeginInfo.renderArea.extent = vkSwapChainExtent;
        
        VkClearValue clearValue{0.0f,0.0f,0.0f,1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(vkCommandBuffer[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(vkCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipeline);
        VkBuffer vertexBuffers[] = {vkVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(vkCommandBuffer[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(vkCommandBuffer[i], vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(vkCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet[i], 0, nullptr);
        vkCmdDrawIndexed(vkCommandBuffer[i], static_cast<uint32_t>(Indexes.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(vkCommandBuffer[i]);
        vkEndCommandBuffer(vkCommandBuffer[i]);
    }
    
    return true;
}

bool Rovski::CreateSyncObjects() {
    vkImageAvailableSemaphore.resize(maxFrameInFlight);
    vkRenderFinishSemaphore.resize(maxFrameInFlight);
    vkInFlightFences.resize(maxFrameInFlight);
    vkImagesInFlight.resize(vkSwapChainImageViews.size(), VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (int i = 0; i < maxFrameInFlight; i++) {
        if (vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, vkImageAvailableSemaphore.data() + i) != VK_SUCCESS
            || vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, vkRenderFinishSemaphore.data() + i) != VK_SUCCESS
            || vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &vkInFlightFences[i]) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}

void Rovski::DrawFrame() {
    uint32_t imageIndex;
    vkWaitForFences(vkDevice, 1, &vkInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(vkDevice, vkSwapChain, UINT64_MAX, vkImageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR){
        RecreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        std::cerr << "failed to acquire image" << std::endl;
        return;
    }
    if (vkImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vkDevice, 1, &vkImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    vkImagesInFlight[imageIndex] = vkInFlightFences[currentFrame];
    UpdateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {vkImageAvailableSemaphore[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = vkCommandBuffer.data() + imageIndex;
    VkSemaphore signalSemaphores[] = {vkRenderFinishSemaphore[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkResetFences(vkDevice, 1, &vkInFlightFences[currentFrame]);
    if (vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkInFlightFences[currentFrame]) != VK_SUCCESS) {
        std::cout << "failed to submit queue" << std::endl;
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {vkSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    result = vkQueuePresentKHR(vkGraphicsQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        RecreateSwapChain();
        frameBufferResized = false;
    } else if (result != VK_SUCCESS) {
        std::cerr << "failed to present" << std::endl;
    }
    currentFrame = (currentFrame+1) % maxFrameInFlight;
}

void Rovski::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(vkDevice);
    CleanUpSwapChain();
    vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, nullptr);
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFrameBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateCommandBuffer();
}

void Rovski::CleanUpSwapChain() {
    for (size_t i = 0; i < vkSwapChainFrameBuffers.size(); i++) {
        vkDestroyFramebuffer(vkDevice, vkSwapChainFrameBuffers[i], nullptr);
    }
    vkFreeCommandBuffers(vkDevice, vkCommandPool, static_cast<uint32_t>(vkCommandBuffer.size()), vkCommandBuffer.data());
    vkDestroyPipeline(vkDevice, vkGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
    vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
    for (size_t i = 0; i < vkSwapChainImageViews.size();i++) {
        vkDestroyImageView(vkDevice, vkSwapChainImageViews[i], nullptr);
        vkDestroyBuffer(vkDevice, vkUniformBuffers[i], nullptr);
        vkFreeMemory(vkDevice, vkUniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, nullptr);
    vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
}

uint32_t Rovski::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);
    for(int i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1<<i) && (properties == (memProperties.memoryTypes[i].propertyFlags & properties))) {
            return i;
        }
    }
    throw(std::runtime_error("failed to right memory type"));
    return 0;
}

bool Rovski::CreateVertexBuffer() {
    VkDeviceSize size = sizeof(Vertices[0]) * Vertices.size();
    VkBuffer stageBuffer;
    VkDeviceMemory stageBufferMemory;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stageBuffer, stageBufferMemory, false);
    void *data;
    vkMapMemory(vkDevice, stageBufferMemory, 0, size, 0, &data);
    memcpy(data, Vertices.data(), size);
    vkUnmapMemory(vkDevice, stageBufferMemory);

    CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vkVertexBuffer, vkVertextBufferMemory, false);
    CopyBuffer(vkVertexBuffer, stageBuffer, size);
    vkDestroyBuffer(vkDevice, stageBuffer, nullptr);
    vkFreeMemory(vkDevice, stageBufferMemory, nullptr);
    return true;
}

bool Rovski::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, bool needTransfer) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    auto indices = FindQueueFamilies(vkPhysicalDevice);
    if (!needTransfer || indices.graphicsFamily.value() == indices.transferFamily.value()) {
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferCreateInfo.queueFamilyIndexCount = 2;
        bufferCreateInfo.pQueueFamilyIndices = std::array<std::remove_pointer<decltype(bufferCreateInfo.pQueueFamilyIndices)>::type, 2>{
                indices.graphicsFamily.value(), indices.transferFamily.value()}.data();
    }
    if(VK_SUCCESS != vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer)){
        return false;
    }
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(vkDevice, buffer, &memoryRequirements);
    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits,
                                                  properties);
    if (vkAllocateMemory(vkDevice, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        return false;
    }
    vkBindBufferMemory(vkDevice, buffer, bufferMemory, 0);
    return true;
}

bool Rovski::CreateIndexBuffer() {
    VkDeviceSize size = sizeof(Indexes[0]) * Indexes.size();
    VkBuffer stageBuffer;
    VkDeviceMemory stageBufferMemory;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stageBuffer, stageBufferMemory, false);
    void *data;
    vkMapMemory(vkDevice, stageBufferMemory, 0, size, 0, &data);
    memcpy(data, Indexes.data(), size);
    vkUnmapMemory(vkDevice, stageBufferMemory);

    CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 vkIndexBuffer, vkIndextBufferMemory, false);
    CopyBuffer(vkIndexBuffer, stageBuffer, size);
    vkDestroyBuffer(vkDevice, stageBuffer, nullptr);
    vkFreeMemory(vkDevice, stageBufferMemory, nullptr);
    return true;
}

bool Rovski::CreateDescriptorLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(vkDevice, &createInfo, nullptr, &vkDescriptorSetLayout)!=VK_SUCCESS) {
        return false;
    }
    return true;
}

bool Rovski::CreateUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    vkUniformBuffers.resize(bufferSize);
    vkUniformBuffersMemory.resize(bufferSize);
    for (int i = 0; i < vkSwapChainImages.size(); i++) {
        if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkUniformBuffers[i], vkUniformBuffersMemory[i], false)){
            return false;
        }
    }
    return true;
}

void Rovski::UpdateUniformBuffer(uint32_t imageIndex) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1), glm::radians(90.0f)*static_cast<float>(currentTimeFromStart), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.prj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkSwapChainExtent.width)/vkSwapChainExtent.height,
                               0.1f, 10.0f);
    ubo.prj[1][1] *= -1;
    void *data;
    vkMapMemory(vkDevice, vkUniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vkDevice, vkUniformBuffersMemory[imageIndex]);
}

void Rovski::UpdateTime() {
    currentTime = std::chrono::high_resolution_clock::now();
    preFrameTimeFromStart = currentTimeFromStart;
    currentTimeFromStart = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();
    deltaTime = currentTimeFromStart - preFrameTimeFromStart;
}

bool Rovski::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize,2> poolSize{};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[0].descriptorCount = static_cast<uint32_t>(vkSwapChainImages.size());
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[1].descriptorCount = static_cast<uint32_t>(vkSwapChainImages.size());

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    createInfo.pPoolSizes = poolSize.data();
    createInfo.maxSets = static_cast<uint32_t>(vkSwapChainImages.size());
    if (VK_SUCCESS != vkCreateDescriptorPool(vkDevice, &createInfo, nullptr, &vkDescriptorPool)) {
        return false;
    }

    return true;
}

bool Rovski::CreateDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layout(vkSwapChainImages.size(), vkDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = vkDescriptorPool;
    allocateInfo.descriptorSetCount = static_cast<uint32_t>(vkSwapChainImages.size());
    allocateInfo.pSetLayouts = layout.data();

    vkDescriptorSet.resize(vkSwapChainImages.size());
    if(vkAllocateDescriptorSets(vkDevice, &allocateInfo, vkDescriptorSet.data()) != VK_SUCCESS) {
        return false;
    }
    for (int i = 0; i < vkSwapChainImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = vkUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = vkTextureImageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = vkTextureSampler;

        std::array<VkWriteDescriptorSet,2> descriptorSet = {};
        descriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorSet[0].dstSet = vkDescriptorSet[i];
        descriptorSet[0].dstBinding = 0;
        descriptorSet[0].dstArrayElement = 0;
        descriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSet[0].descriptorCount = 1;
        descriptorSet[0].pBufferInfo = &bufferInfo;
        descriptorSet[0].pImageInfo = nullptr;
        descriptorSet[0].pTexelBufferView = nullptr;

        descriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorSet[1].dstSet = vkDescriptorSet[i];
        descriptorSet[1].dstBinding = 1;
        descriptorSet[1].dstArrayElement = 0;
        descriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorSet[1].descriptorCount = 1;
        descriptorSet[1].pBufferInfo = &bufferInfo;
        descriptorSet[1].pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(vkDevice, static_cast<uint32_t>(descriptorSet.size()), descriptorSet.data(), 0, nullptr);
    }
    return true;
}

bool Rovski::CreateTextureImage() {
    int texHeight, texWidth, texChannels;
    stbi_uc* pixels = stbi_load("/Users/luobin/Rovski/Rovski/Texture/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        std::cout << __LINE__ << std::endl;
        return false;
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingMemory, false);
    void *data;
    vkMapMemory(vkDevice, stagingMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(vkDevice, stagingMemory);
    stbi_image_free(pixels);

    if (!CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkTextureImage,
                             vkTextureMemory)) {
        std::cout << __LINE__ << std::endl;
        return false;
    }
    TransitionImageLayout(vkTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    std::cout << texHeight << "x" << texWidth << "=" << texHeight * texWidth << ", channels: " << texChannels << std::endl;
    CopyBufferToImage(stagingBuffer, vkTextureImage, texWidth, texHeight);
    TransitionImageLayout(vkTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingMemory, nullptr);
    return true;
}

bool Rovski::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkImage &image,
                         VkDeviceMemory &imageMemory) {
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.extent.depth = 1;
    createInfo.extent.width = width;
    createInfo.extent.height = height;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.format = format;
    createInfo.tiling = tiling;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.flags = 0;
    if (vkCreateImage(vkDevice, &createInfo, nullptr, &image) != VK_SUCCESS) {
        std::cout << __LINE__ << std::endl;
        return false;
    }
    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(vkDevice, vkTextureImage, &memoryRequirements);
    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);
    if (VK_SUCCESS != vkAllocateMemory(vkDevice, &allocateInfo, nullptr, &imageMemory)) {
        std::cout << __LINE__ << std::endl;
        return false;
    }
    vkBindImageMemory(vkDevice, vkTextureImage, vkTextureMemory, 0);
    return true;
}

VkCommandBuffer Rovski::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.commandPool = vkCommandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vkDevice, &allocateInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Rovski::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkGraphicsQueue);
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);
}

bool Rovski::CopyBuffer(VkBuffer &dst, VkBuffer &src, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferCopy region{};
    region.size = size;
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &region);
    EndSingleTimeCommands(commandBuffer);
    return true;
}

void Rovski::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    VkPipelineStageFlags srcStage, dstStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        std::cout << "Invalid transition type!" << std::endl;
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndSingleTimeCommands(commandBuffer);
}

void Rovski::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferImageHeight = 0;
    region.bufferRowLength = 0;
    region.bufferOffset = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageOffset = {0,0,0};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommands(commandBuffer);
}

bool Rovski::CreateTextureImageView() {
    vkTextureImageView = CreateImageView(vkTextureImage, VK_FORMAT_R8G8B8A8_SRGB);
    if (vkTextureImageView == VK_NULL_HANDLE) {
        return false;
    }
    return true;
}
VkImageView Rovski::CreateImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.layerCount = 1;
    VkImageView imageView;
    if (VK_SUCCESS != vkCreateImageView(vkDevice, &createInfo, nullptr, &imageView)) {
        return VK_NULL_HANDLE;
    }
    return imageView;
}

bool Rovski::CreateTextureSampler() {
    VkSamplerCreateInfo createInfo{};
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    if (vkDeviceFeatures.samplerAnisotropy == VK_TRUE) {
        createInfo.anisotropyEnable = VK_TRUE;
        createInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
    } else {
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 0.0f;
    }
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;
    if (VK_SUCCESS != vkCreateSampler(vkDevice, &createInfo, nullptr, &vkTextureSampler)) {
        return false;
    }

    return true;
}