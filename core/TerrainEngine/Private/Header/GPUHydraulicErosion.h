#pragma once
#ifdef TE_USE_GPU
#include "HydraulicErosion.h"
#include <vulkan/vulkan.h>

struct FDeviceBackedBuffer
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};

class CGPUHydraulicErosion : public CHydraulicErosion
{
    private:
        /*
        In order to use Vulkan, you must create an instance. 
        */
        VkInstance m_vkInstance;

        VkDebugReportCallbackEXT debugReportCallback;
        /*
        The physical device is some device on the system that supports usage of Vulkan.
        Often, it is simply a graphics card that supports Vulkan. 
        */
        VkPhysicalDevice m_vkPhyDevice;
        /*
        Then we have the logical device VkDevice, which basically allows 
        us to interact with the physical device. 
        */
        VkDevice m_vkDevice;;

        /*
        The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
        We will be creating a simple compute pipeline in this application. 
        */
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkShaderModule computeShaderModule;

        /*
        The command buffer is used to record commands, that will be submitted to a queue.
        To allocate such command buffers, we use a command pool.
        */
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        /*
        Descriptors represent resources in shaders. They allow us to use things like
        uniform buffers, storage buffers and images in GLSL. 
        A single descriptor represents a single resource, and several descriptors are organized
        into descriptor sets, which are basically just collections of descriptors.
        */
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;

        /*
            Buffers
        */
        // VkBuffer waterMapBuffer;
        // VkDeviceMemory waterMapBufferMemory;

        // VkBuffer sedimentMapBuffer;
        // VkDeviceMemory sedimentMapBufferMemory;

        // VkBuffer m_heightMapBuffer;
        // VkDeviceMemory m_heightMapBufferMemory;

        // VkBuffer accumulationMapBuffer;
        // VkDeviceMemory accumulationMapBufferMemory;

        // VkBuffer outMapBuffer;
        // VkDeviceMemory outMapBufferMemory;


        // Enabled Layers

        std::vector<const char *> enabledLayers;

        /*
        In order to execute commands on a device(GPU), the commands must be submitted
        to a queue. The commands are stored in a command buffer, and this command buffer
        is given to the queue. 
        There will be different kinds of queues on the device. Not all queues support
        graphics operations, for instance. For this application, we at least want a queue
        that supports compute operations. 
        */
        VkQueue queue; // a queue supporting compute operations.

        /*
        Groups of queues that have the same capabilities(for instance, they all supports graphics and computer operations),
        are grouped into queue families. 
        
        When submitting a command buffer, you must specify to which queue in the family you are submitting to. 
        This variable keeps track of the index of that queue in its family. 
        */
        uint32_t queueFamilyIndex;

    private:
        void CreateVKInstance();
        void CreateDevice();
        void CreatePhysicalDevice();
        void CreateBuffer(FDeviceBackedBuffer &stBuffer, uint64_t uSize);
        void CreateDescSetLayout();
        void CreateDescSet();
        void CreateComputePipe();
        void CreateCommandBuffer();

        void DeleteBuffer(FDeviceBackedBuffer &stBuffer);

    public:
        CGPUHydraulicErosion(int32_t iOverscan, uint32_t uSeed, int32_t iOffsetX, int32_t iOffsetY, FLOAT_TYPE fWaterLevel = 0.1f);
        ~CGPUHydraulicErosion();

        virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f) override;
        virtual void ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f) override;
        //virtual void Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale = 0.1f);
};


#endif