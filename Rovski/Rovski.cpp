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

const std::vector<const char*> Rovski::validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


const std::vector<const char*> Rovski::deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Rovski::Rovski(){
    
}

Rovski::~Rovski(){
    
}

void Rovski::Run(){
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

bool Rovski::Init(uint32_t windowWidth, uint32_t windowHeight) {
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
    InitWindow();
    InitVulkan();
    return true;
}

bool Rovski::Clean(){
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
    for (auto view : swapChainImageViews) {
        vkDestroyImageView(vkDevice, view, nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(windowWidth, windowHeight, "Rovski", nullptr, nullptr);
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
    if(!CreateGraphicsPipeline()) {
        std::cout << "failed to create graphics pipeline" << std::endl;
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
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
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
        return true;
    }
    return false;
}

int Rovski::RateDevice(VkPhysicalDevice device){
    int score = 0;
    VkPhysicalDeviceProperties devicePropoerties;
    vkGetPhysicalDeviceProperties(device, &devicePropoerties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    if (devicePropoerties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100;
    }
    if (deviceFeatures.geometryShader) {
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
    
    std::set<uint32_t> uniqueQueueFamilies = {queueFamily.graphicsFamily.value(), queueFamily.presentFamily.value()};
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
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
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
    if (indices.graphicsFamily != indices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndeices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
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
    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(vkDevice, vkSwapChain, &swapChainImageCount, swapChainImages.data());
    vkSwapChainFormat = format.format;
    vkSwapChainExtent = extent;
    return true;
}

bool Rovski::CreateImageViews(){
    swapChainImageViews.resize(swapChainImages.size());
    for (int i = 0; i < swapChainImages.size(); i++) {
        auto &image = swapChainImages[i];
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vkSwapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(vkDevice, &createInfo, nullptr, swapChainImageViews.data() + i);
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
    
    VkPipelineShaderStageCreateInfo vertCreateInfo;
    vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertCreateInfo.module = vertShaderModule;
    vertCreateInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragCreateInfo;
    fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    fragCreateInfo.module = fragShaderModule;
    fragCreateInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertCreateInfo, fragCreateInfo};
    
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
    
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
    viewportStateCreateInfo.scissorCount = 0;
    viewportStateCreateInfo.pScissors = &scissor;
    viewportStateCreateInfo.viewportCount = 0;
    viewportStateCreateInfo.pViewports = &viewPort;
    
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_TRUE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    
    if(vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
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
    
    return true;
}
