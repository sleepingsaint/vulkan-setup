#include <math.h>
#include <fstream>
#include <iostream>

#include <VkEngine.h>
#include <VkBootstrap.h>
#include <VkInitializers.h>

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

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    
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

bool VulkanEngine::loadShaderModule(const char* filePath, VkShaderModule &shaderModule){
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo = vkInit::createShaderModuleCreateInfo(buffer.size() * sizeof(uint32_t), buffer.data());
    if(vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS){
        throw std::runtime_error("Unable to load shader module");
        return false;
    }
    return true;
}

void VulkanEngine::initPipelines(){
    VkShaderModule triangleFragShader;
	if (!loadShaderModule("../shaders/triangle.frag.spv", triangleFragShader)){
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}else {
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
	}

	VkShaderModule triangleVertexShader;
	if (!loadShaderModule("../shaders/triangle.vert.spv", triangleVertexShader)){
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}else {
		std::cout << "Triangle vertex shader successfully loaded" << std::endl;
	}

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkInit::createPipelineLayoutCreateInfo();
	if(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_trianglePipelineLayout) != VK_SUCCESS){
        std::runtime_error("Unable to create pipeline layout");
    }

    PipelineBuilder pipelineBuilder = PipelineBuilder();
    pipelineBuilder._shaderStages.push_back(
        vkInit::createPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader)
    );
    pipelineBuilder._shaderStages.push_back(
        vkInit::createPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader)
    );

    pipelineBuilder._vertexInputInfo = vkInit::createPipelineVertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkInit::createPipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)_extent.width;
	pipelineBuilder._viewport.height = (float)_extent.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = _extent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkInit::createPipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we don't use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkInit::createPipelineMultiSampleStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkInit::createPipelineColorBlendAttachmentState();

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

	//finally build the pipeline
	_trianglePipeline = pipelineBuilder.buildPipeline(_device, _renderPass);
}
void VulkanEngine::init(){
    initSurface();
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderPass();
    initFrameBuffers();
    initSyncStructures();
    initPipelines();
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

