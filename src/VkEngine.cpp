#include <math.h>
#include <fstream>
#include <iostream>

#include <VkEngine.h>
#include <VkBootstrap.h>
#include <VkInitializers.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <glm/gtx/transform.hpp>

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

    VmaAllocatorCreateInfo info = {};
    info.physicalDevice = _physicalDevice;
    info.device = _device;
    info.instance = _instance;
    if(vmaCreateAllocator(&info, &_allocator) != VK_SUCCESS){
        throw std::runtime_error("Unable to create allocator");
    }
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

    _deletionQueue.push_function([=](){
        vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    });
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

    _deletionQueue.push_function([=]() {
        vkDestroyCommandPool(_device, _commandPool, nullptr);
    });
}

void VulkanEngine::initDefaultRenderPass(){
    VkAttachmentDescription colorAttachment = vkInit::createAttachmentDescription(_swapChainImageFormat);
    VkAttachmentReference colorAttachmentRef = vkInit::createAttachmentReference(0);
    VkSubpassDescription subpassDescription = vkInit::createSubPassDescription(1, colorAttachmentRef);
    VkRenderPassCreateInfo renderPassInfo = vkInit::createRenderPassInfo(1, colorAttachment, 1, subpassDescription);
    if(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS){
        throw std::runtime_error("Unable to create default render pass");
    }

    _deletionQueue.push_function([=]() {
        vkDestroyRenderPass(_device, _renderPass, nullptr);
    });
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

        _deletionQueue.push_function([=]() {
            vkDestroyFramebuffer(_device, _frameBuffers[i], nullptr);
            vkDestroyImageView(_device, _swapChainImageViews[i], nullptr);
        });
    }
}

void VulkanEngine::initSyncStructures(){
    VkFenceCreateInfo fenceCreateInfo = vkInit::createFenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    if(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence) != VK_SUCCESS){
        throw std::runtime_error("Unable to create render fence");
    }

    _deletionQueue.push_function([=](){
        vkDestroyFence(_device, _renderFence, nullptr);
    });

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkInit::createSemaphoreCreateInfo(0);
    if(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore) != VK_SUCCESS){
        throw std::runtime_error("Unable to create Render Semaphore");
    }
    if(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore) != VK_SUCCESS){
        throw std::runtime_error("Unable to create Present Semaphore");
    }

    _deletionQueue.push_function([=]() {
        vkDestroySemaphore(_device, _renderSemaphore, nullptr);
        vkDestroySemaphore(_device, _presentSemaphore, nullptr);
    });
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

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);

	//bind the mesh vertex buffer with offset 0
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_triangleMesh._vertexBuffer._buffer, &offset);
    
    glm::vec3 camPos = { 0.f,0.f,-2.f };

    glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
    //camera projection
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;
    //model rotation
    glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(_frameNumber * 0.4f), glm::vec3(0, 1, 0));

    //calculate final mesh matrix
    glm::mat4 mesh_matrix = projection * view * model;

    MeshPushConstants constants;
    constants.renderMatrix = mesh_matrix;

    //upload the matrix to the GPU via push constants
    vkCmdPushConstants(cmd, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
	//we can now draw the mesh
	vkCmdDraw(cmd, _triangleMesh._vertices.size(), 1, 0, 0);
    
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
	if (!loadShaderModule("../shaders/colored_triangle.frag.spv", triangleFragShader)){
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}else {
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
	}

	VkShaderModule triangleVertexShader;
	if (!loadShaderModule("../shaders/colored_triangle.vert.spv", triangleVertexShader)){
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}else {
		std::cout << "Triangle vertex shader successfully loaded" << std::endl;
	}

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkInit::createPipelineLayoutCreateInfo();
	if(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_trianglePipelineLayout) != VK_SUCCESS){
        std::runtime_error("Unable to create pipeline layout");
    }
    
    VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkInit::createPipelineLayoutCreateInfo();
    VkPushConstantRange pushConstantRange = vkInit::createPushConstantRange(sizeof(MeshPushConstants), VK_SHADER_STAGE_VERTEX_BIT);
    meshPipelineLayoutInfo.pushConstantRangeCount = 1;
    meshPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if(vkCreatePipelineLayout(_device, &meshPipelineLayoutInfo, nullptr, &_meshPipelineLayout) != VK_SUCCESS){
        std::runtime_error("Unable to create mesh pipeline layout");
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

    VertexInputDescription vertexDescription = Vertex::getVertexDescription();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	//clear the shader stages for the builder
	pipelineBuilder._shaderStages.clear();

	//compile mesh vertex shader
	VkShaderModule meshVertShader;
	if (!loadShaderModule("../shaders/triangle_mesh.vert.spv", meshVertShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else {
		std::cout << "mesh Triangle vertex shader successfully loaded" << std::endl;
	}

    pipelineBuilder._shaderStages.push_back(
		vkInit::createPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

	//make sure that triangleFragShader is holding the compiled colored_triangle.frag
	pipelineBuilder._shaderStages.push_back(
		vkInit::createPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

	//build the mesh triangle pipeline
    pipelineBuilder._pipelineLayout = _meshPipelineLayout;
	_meshPipeline = pipelineBuilder.buildPipeline(_device, _renderPass);

    vkDestroyShaderModule(_device, triangleVertexShader, nullptr);
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);
    vkDestroyShaderModule(_device, meshVertShader, nullptr);

    _deletionQueue.push_function([=]() {
        vkDestroyPipeline(_device, _trianglePipeline, nullptr);
        vkDestroyPipeline(_device, _meshPipeline, nullptr);
        vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
        vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);
    });
}

void VulkanEngine::loadMeshes(){
    _triangleMesh._vertices.resize(3);

	//vertex positions
	_triangleMesh._vertices[0].position = { 1.f, 1.f, 0.0f };
	_triangleMesh._vertices[1].position = {-1.f, 1.f, 0.0f };
	_triangleMesh._vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, all green
	_triangleMesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangleMesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	_triangleMesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

	//we don't care about the vertex normals
	uploadMesh(_triangleMesh);
}

void VulkanEngine::uploadMesh(Mesh &mesh){
    VkBufferCreateInfo bufferCreateInfo = vkInit::createBufferCreateInfo(
        mesh._vertices.size() * sizeof(Vertex), 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    if(vmaCreateBuffer(
        _allocator, 
        &bufferCreateInfo, 
        &vmaAllocInfo, 
        &mesh._vertexBuffer._buffer,
        &mesh._vertexBuffer._allocation,
        nullptr) != VK_SUCCESS){
            throw std::runtime_error("Unable to allocate buffer");
        }
    _deletionQueue.push_function([=]() {
        vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
    });

    void* data;
	vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);
	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));
	vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
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

    loadMeshes();
}

void VulkanEngine::mainLoop(){
    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();
        draw();
    }
}

void VulkanEngine::cleanUp(){
    vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);
    _deletionQueue.flush();
    vmaDestroyAllocator(_allocator);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    
    vkDestroyDevice(_device, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
    vkDestroyInstance(_instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

