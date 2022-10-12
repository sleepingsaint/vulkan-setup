#define GLFW_INCLUDE_VULKAN
#define VMA_IMPLEMENTATION

#include <iostream>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <VkEngine.h>

int main(){
    VulkanEngine engine = VulkanEngine();
    engine.init();
    engine.mainLoop();
    engine.cleanUp();

    std::cout << "Hello world" << std::endl;
    return 0;
}