#include <VkEngine.h>
#include <VkBootstrap.h>
#include <VkInitializers.h>
#include <math.h>

void VulkanEngine::initSurface(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Title", nullptr, nullptr);
    _extent = {
        static_cast<uint32_t>(WIDTH),
        static_cast<uint32_t>(HEIGHT)
    };
}

void VulkanEngine::initVulkan(){
    vkb::InstanceBuilder builder;
    auto inst_ret = builder
        .set_app_name("Vulkan Shader application")
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(1, 1, 0)
        .build();
    
    vkb::Instance vkb_inst = inst_ret.value();
    _instance = vkb_inst.instance;
    _debugMessenger = vkb_inst.debug_messenger;

    if(glfwCreateWindowSurface(_instance, window, nullptr, &_surface) != VK_SUCCESS){
        throw std::runtime_error("Failed to create window surface");
    }

    vkb::PhysicalDeviceSelector selector { vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 1)
        .set_surface(_surface)
        .select()
        .value();
    
    vkb::DeviceBuilder deviceBuilder {physicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _physicalDevice = vkbDevice.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

}

void VulkanEngine::initSwapchain(){
    vkb::SwapchainBuilder swapChainBuilder(_physicalDevice, _device, _surface);
    vkb::Swapchain vkbSwapChain = swapChainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(_extent.width, _extent.height)
        .build()
        .value();

    _swapChain = vkbSwapChain.swapchain;
    _swapChainImageFormat = vkbSwapChain.image_format;
    _swapChainImages = vkbSwapChain.get_images().value();
    _swapChainImageViews = vkbSwapChain.get_image_views().value();
}

void VulkanEngine::initCommands(){
    VkCommandPoolCreateInfo createInfo = vkInit::createCommandPoolCreateInfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    if(vkCreateCommandPool(_device, &createInfo, nullptr, &_commandPool) != VK_SUCCESS){
        throw std::runtime_error("Unable to create command pool");
    }

    VkCommandBufferAllocateInfo cmdBufAlocInfo = vkInit::createCommandBufferAllocateInfo(_commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    if(vkAllocateCommandBuffers(_device, &cmdBufAlocInfo, &_mainCommandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Unable to allocate command buffers");
    }
}

void VulkanEngine::initDefaultRenderPass(){
    VkAttachmentDescription colorAttachment = vkInit::createAttachmentDescription(_swapChainImageFormat);
    VkAttachmentReference colorAttachmentRef = vkInit::createAttachmentReference(0);
    VkSubpassDescription subpassDescription = vkInit::createSubPassDescription(1, colorAttachmentRef);
    VkRenderPassCreateInfo renderPassInfo = vkInit::createRenderPassInfo(1, colorAttachment, 1, subpassDescription);
    if(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS){
        throw std::runtime_error("Unable to create default render pass");
    }
}

void VulkanEngine::initFrameBuffers(){
    VkFramebufferCreateInfo createInfo = vkInit::createFrameBufferCreateInfo(_renderPass, 1, 1, _extent);
    const uint32_t swapChainImageCount = _swapChainImages.size();
    _frameBuffers = std::vector<VkFramebuffer>(swapChainImageCount);
    for(uint32_t i = 0; i < swapChainImageCount; i++){
        createInfo.pAttachments = &_swapChainImageViews[i];
        if(vkCreateFramebuffer(_device, &createInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Unable to create frame buffers");
        }
    }
}

void VulkanEngine::initSyncStructures(){
    VkFenceCreateInfo fenceCreateInfo = vkInit::createFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    if(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence) != VK_SUCCESS){
        throw std::runtime_error("Unable to create render fence");
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkInit::createSemaphoreCreateInfo(0);
    if(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore) != VK_SUCCESS){
        throw std::runtime_error("Unable to create Render Semaphore");
    }
    if(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore) != VK_SUCCESS){
        throw std::runtime_error("Unable to create Present Semaphore");
    }
}

void VulkanEngine::draw(){
    if(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000) != VK_SUCCESS){
        throw std::runtime_error("Unable to wait for fences");
    }
    if(vkResetFences(_device, 1, &_renderFence) != VK_SUCCESS){
        throw std::runtime_error("Unable to reset the render fence");
    }

    uint32_t swapChainImageIndex;
    if(vkAcquireNextImageKHR(_device, _swapChain, 1000000000, _presentSemaphore, nullptr, &swapChainImageIndex) != VK_SUCCESS){
        throw std::runtime_error("Unable to acquire next image");
    }

    if(vkResetCommandBuffer(_mainCommandBuffer, 0) != VK_SUCCESS){
        throw std::runtime_error("Unable to reset command buffer");
    }

    VkCommandBuffer cmd = _mainCommandBuffer;
    VkCommandBufferBeginInfo cmdBeginInfo = vkInit::createCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    if(vkBeginCommandBuffer(cmd, &cmdBeginInfo) != VK_SUCCESS){
        throw std::runtime_error("Unable to begin commmands");
    }

    VkClearValue clearValue;
	float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

    VkRenderPassBeginInfo renderPassInfo = vkInit::createRenderPassBeginInfo(_renderPass, 0, 0, _extent, _frameBuffers[swapChainImageIndex], 1, clearValue);
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = vkInit::createSubmitInfo(waitStage, 1, _presentSemaphore, 1, _renderSemaphore, 1, cmd);
    if(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _renderFence) != VK_SUCCESS){
        throw std::runtime_error("Unable to submit cmd buffer");
    }

    VkPresentInfoKHR presentInfo = vkInit::createPresentInfoKHR(1, _swapChain, 1, _renderSemaphore, swapChainImageIndex);
    vkQueuePresentKHR(_graphicsQueue, &presentInfo);
    _frameNumber++;
}

void VulkanEngine::init(){
    initSurface();
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderPass();
    initFrameBuffers();
    initSyncStructures();
}

void VulkanEngine::mainLoop(){
    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();
        draw();
    }
}

void VulkanEngine::cleanUp(){
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);
    for(int i = 0; i < _frameBuffers.size(); i++){
        vkDestroyFramebuffer(_device, _frameBuffers[i], nullptr);
        vkDestroyImageView(_device, _swapChainImageViews[i], nullptr);
    }
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}