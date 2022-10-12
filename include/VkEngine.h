#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanEngine {
public:
    GLFWwindow* window;

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkSurfaceKHR _surface;
    VkExtent2D _extent;

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;

    VkSwapchainKHR _swapChain;
    VkFormat _swapChainImageFormat;
    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;

    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    VkRenderPass _renderPass;
    std::vector<VkFramebuffer> _frameBuffers;

    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    int _frameNumber = 0;

    void init();
    void draw();
    void mainLoop();
    void cleanUp();

private:
    void initSurface();
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultRenderPass();
    void initFrameBuffers();
    void initSyncStructures();
};