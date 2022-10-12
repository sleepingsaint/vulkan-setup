#include <vulkan/vulkan.h>

namespace vkInit {
    VkCommandPoolCreateInfo createCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);

    VkCommandBufferAllocateInfo createCommandBufferAllocateInfo(VkCommandPool &commandPool, uint32_t bufferCount, VkCommandBufferLevel level);

    VkAttachmentDescription createAttachmentDescription(VkFormat format);
    
    VkAttachmentReference createAttachmentReference(uint32_t attachmentIndex);

    VkSubpassDescription createSubPassDescription(uint32_t attachmentCount, VkAttachmentReference &ref);

    VkRenderPassCreateInfo createRenderPassInfo(uint32_t attachmentCount, VkAttachmentDescription &attachment, uint32_t subPassCount, VkSubpassDescription &subpassDescription);

    VkFramebufferCreateInfo createFrameBufferCreateInfo(VkRenderPass &renderPass, uint32_t attachmentCount, uint32_t numLayers, VkExtent2D &extent);

    VkFenceCreateInfo createFenceCreateInfo(VkFenceCreateFlags flags);

    VkSemaphoreCreateInfo createSemaphoreCreateInfo(VkSemaphoreCreateFlags flags);

    VkCommandBufferBeginInfo createCommandBufferBeginInfo(VkCommandBufferUsageFlags flags);

    VkRenderPassBeginInfo createRenderPassBeginInfo(VkRenderPass &renderPass, uint32_t offsetX, uint32_t offsetY, VkExtent2D &extent, VkFramebuffer &frameBuffer, uint32_t clearValueCount, VkClearValue &clearValue);

    VkSubmitInfo createSubmitInfo(VkPipelineStageFlags &waitStage,
        uint32_t presentSemaphoreCount,
        VkSemaphore &presentSemaphore,
        uint32_t renderSemaphoreCount,
        VkSemaphore &renderSemaphore,
        uint32_t bufferCount,
        VkCommandBuffer &cmdBuffer
    );
    
    VkPresentInfoKHR createPresentInfoKHR(
        uint32_t swapChainCount,
        VkSwapchainKHR &swapChain, 
        uint32_t renderSemaphoreCount, 
        VkSemaphore &renderSemaphore,
        uint32_t &imageIndex
    );
}