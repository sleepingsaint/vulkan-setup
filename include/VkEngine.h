#include <deque>
#include <vector>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkPipelineBuilder.h>
#include <vk_mem_alloc.h>
#include <VkMesh.h>

#include <glm/glm.hpp>

struct MeshPushConstants{
    glm::vec4 data;
    glm::mat4 renderMatrix;
};

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}

		deletors.clear();
	}
};

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

    VkPipelineLayout _trianglePipelineLayout;
    VkPipeline _trianglePipeline;
    
    VkPipeline _meshPipeline;
    Mesh _triangleMesh;

    VmaAllocator _allocator;
    DeletionQueue _deletionQueue;

    VkPipelineLayout _meshPipelineLayout;

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
    void initPipelines();

    bool loadShaderModule(const char* filePath, VkShaderModule &shaderModule);

    void loadMeshes();
    void uploadMesh(Mesh &mesh);
};