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
