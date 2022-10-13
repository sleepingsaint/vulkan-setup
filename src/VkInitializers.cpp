#include <VkInitializers.h>

VkCommandPoolCreateInfo vkInit::createCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags){
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;

    return info;
}

VkCommandBufferAllocateInfo vkInit::createCommandBufferAllocateInfo(VkCommandPool &commandPool, uint32_t bufferCount, VkCommandBufferLevel level){
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;

    info.commandPool = commandPool;
    info.commandBufferCount = bufferCount;
    info.level = level;
    return info;
}

VkAttachmentDescription vkInit::createAttachmentDescription(VkFormat format){
    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return attachment;
}

VkAttachmentReference vkInit::createAttachmentReference(uint32_t attachment_index){
    VkAttachmentReference ref = {};
    ref.attachment = attachment_index;
    ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    return ref;
}

VkSubpassDescription vkInit::createSubPassDescription(uint32_t attachmentCount, VkAttachmentReference &ref){
    VkSubpassDescription description = {};
    description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    description.colorAttachmentCount = attachmentCount;
    description.pColorAttachments = &ref;
    return description;
}

VkRenderPassCreateInfo vkInit::createRenderPassInfo(uint32_t attachmentCount, VkAttachmentDescription &attachment, uint32_t subPassCount, VkSubpassDescription &subpassDescription){
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;

    info.subpassCount = subPassCount;
    info.pSubpasses = &subpassDescription;
    return info;
}

VkFramebufferCreateInfo vkInit::createFrameBufferCreateInfo(VkRenderPass &renderPass, uint32_t attachmentCount, uint32_t numLayers, VkExtent2D &extent){
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext = nullptr;
    info.renderPass = renderPass;
    info.attachmentCount = attachmentCount;
    info.width = extent.width;
    info.height = extent.height;
    info.layers = numLayers;
    return info;
}

VkFenceCreateInfo vkInit::createFenceCreateInfo(VkFenceCreateFlags flags){
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

VkSemaphoreCreateInfo vkInit::createSemaphoreCreateInfo(VkSemaphoreCreateFlags flags){
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

VkCommandBufferBeginInfo vkInit::createCommandBufferBeginInfo(VkCommandBufferUsageFlags flags){
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkRenderPassBeginInfo vkInit::createRenderPassBeginInfo(VkRenderPass &renderPass, uint32_t offsetX, uint32_t offsetY, VkExtent2D &extent, VkFramebuffer &frameBuffer, uint32_t clearValueCount, VkClearValue &clearValue){
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.pNext = nullptr;

    info.renderPass = renderPass;
    info.renderArea.offset.x = offsetX;
    info.renderArea.offset.y = offsetY;
    info.renderArea.extent = extent;
    info.framebuffer = frameBuffer;

    info.clearValueCount = clearValueCount;
    info.pClearValues = &clearValue;

    return info;
}

VkSubmitInfo vkInit::createSubmitInfo(
    VkPipelineStageFlags &waitStage,
    uint32_t presentSemaphoreCount,
    VkSemaphore &presentSemaphore,
    uint32_t renderSemaphoreCount,
    VkSemaphore &renderSemaphore,
    uint32_t bufferCount,
    VkCommandBuffer &cmdBuffer
){
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;

    info.pWaitDstStageMask = &waitStage;

    info.waitSemaphoreCount = presentSemaphoreCount;
    info.pWaitSemaphores = &presentSemaphore;

    info.signalSemaphoreCount = renderSemaphoreCount;
    info.pSignalSemaphores = &renderSemaphore;

    info.commandBufferCount = bufferCount;
    info.pCommandBuffers = &cmdBuffer;

    return info;
}

VkPresentInfoKHR vkInit::createPresentInfoKHR(
    uint32_t swapChainCount,
    VkSwapchainKHR &swapChain, 
    uint32_t renderSemaphoreCount, 
    VkSemaphore &renderSemaphore,
    uint32_t &imageIndex
){
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext = nullptr;

    info.swapchainCount = swapChainCount;
    info.pSwapchains = &swapChain;
    
    info.waitSemaphoreCount = renderSemaphoreCount;
    info.pWaitSemaphores = &renderSemaphore;

    info.pImageIndices = &imageIndex;

    return info; 
}

VkShaderModuleCreateInfo vkInit::createShaderModuleCreateInfo(size_t size, const uint32_t *data){
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.pNext = nullptr;
    info.codeSize = size;
    info.pCode = data;
    return info;
}

VkPipelineShaderStageCreateInfo vkInit::createPipelineShaderStageCreateInfo(VkShaderStageFlagBits stageBits, VkShaderModule &shaderModule){
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.stage = stageBits;
    info.module = shaderModule;
    info.pName = "main";
    return info;
}

VkPipelineVertexInputStateCreateInfo vkInit::createPipelineVertexInputStateCreateInfo() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    //no vertex bindings or attributes
    info.vertexBindingDescriptionCount = 0;
    info.vertexAttributeDescriptionCount = 0;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo vkInit::createPipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    //we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}

VkPipelineRasterizationStateCreateInfo vkInit::createPipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode){
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;

    info.polygonMode = polygonMode;
    info.lineWidth = 1.0f;
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;

    return info;
}

VkPipelineMultisampleStateCreateInfo vkInit::createPipelineMultiSampleStateCreateInfo(){
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    //multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendAttachmentState vkInit::createPipelineColorBlendAttachmentState() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vkInit::createPipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    //empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

