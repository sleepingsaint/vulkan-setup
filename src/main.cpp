#include <VkEngine.h>

int main(){
    VulkanEngine engine = VulkanEngine();
    engine.init();
    engine.mainLoop();
    engine.cleanUp();

    return 0;
}