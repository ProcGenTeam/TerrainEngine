#include <vulkan/vulkan_core.h>
#ifdef TE_USE_GPU
#include "core/TerrainEngine/Private/Header/GPUHydraulicErosion.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <stdexcept>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define VK_CHECK_RESULT(f)                                                                                             \
    {                                                                                                                  \
        VkResult res = (f);                                                                                            \
        if (res != VK_SUCCESS)                                                                                         \
        {                                                                                                              \
            printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, __LINE__);                              \
            assert(res == VK_SUCCESS);                                                                                 \
        }                                                                                                              \
    }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(VkDebugReportFlagsEXT flags,
                                                            VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                            size_t location, int32_t messageCode,
                                                            const char *pLayerPrefix, const char *pMessage,
                                                            void *pUserData)
{
    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);
    return VK_FALSE;
}

CGPUHydraulicErosion::CGPUHydraulicErosion(int32_t iOverscan, uint32_t uSeed, int32_t iOffsetX, int32_t iOffsetY,
                                           FLOAT_TYPE fWaterLevel)
    : CHydraulicErosion(iOverscan, uSeed, iOffsetX, iOffsetY, fWaterLevel)
{
    CreateVKInstance();
    CreatePhysicalDevice();
    CreateDevice();
    // CreateBuffer(m_heightMapBuffer, m_heightMapBufferMemory, );
    CreateShaderModules("Erode.spv");
    CreateShaderModules("Combine.spv");
    CreateShaderModules("Rain.spv");
    CreateShaderModules("Dump.spv");
    CreateShaderModules("Zero.spv");
    // printf("Creating Desc Set Layout\n");
    CreateDescSetLayout();
    // printf("Creating Pipe\n");
    CreateComputePipe();
    // printf("Creating Desc Set\n");
    CreateDescSet();
    // printf("Creating Command Buffer\n");
    CreateCommandBuffer();
}

CGPUHydraulicErosion::~CGPUHydraulicErosion()
{
    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &m_vkCommandBuffer);

    for (auto pipe : m_vPipelines)
    {
        vkDestroyPipeline(m_vkDevice, pipe.vkPipeline, NULL);
        vkDestroyPipelineLayout(m_vkDevice, pipe.vkPipelineLayout, NULL);
    }

    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, NULL);
    vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, NULL);
    vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, NULL);

    for (auto shader : m_vShaders)
    {
        vkDestroyShaderModule(m_vkDevice, shader.vkShaderModule, NULL);
    }
    vkDestroyDevice(m_vkDevice, NULL);
    vkDestroyInstance(m_vkInstance, NULL);
}

void CGPUHydraulicErosion::CreateVKInstance()
{
    std::vector<const char *> enabledExtensions;

    /*
        By enabling validation layers, Vulkan will emit warnings if the API
        is used incorrectly. We shall enable the layer VK_LAYER_LUNARG_standard_validation,
        which is basically a collection of several useful validation layers.
        */
    if (enableValidationLayers)
    {
        /*
                We get all supported layers with vkEnumerateInstanceLayerProperties.
                */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, NULL);

        std::vector<VkLayerProperties> layerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        /*
                And then we simply check if VK_LAYER_LUNARG_standard_validation is among the supported layers.
                */
        bool foundLayer = false;
        for (VkLayerProperties prop : layerProperties)
        {

            if (strcmp("VK_LAYER_LUNARG_standard_validation", prop.layerName) == 0)
            {
                foundLayer = true;
                break;
            }
        }

        if (!foundLayer)
        {
            throw std::runtime_error("Layer VK_LAYER_LUNARG_standard_validation not supported\n");
        }
        enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation"); // Alright, we can use this layer.

        /*
                We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                in order to be able to print the warnings emitted by the validation layer.
                So again, we just check if the extension is among the supported extensions.
                */

        uint32_t extensionCount;

        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        std::vector<VkExtensionProperties> extensionProperties(extensionCount);
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

        bool foundExtension = false;
        for (VkExtensionProperties prop : extensionProperties)
        {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0)
            {
                foundExtension = true;
                break;
            }
        }

        if (!foundExtension)
        {
            throw std::runtime_error("Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
        }
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // uint32_t extensionCount;

    // vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    // std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    // vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data());

    // bool foundExtension = false;
    // for (VkExtensionProperties prop : extensionProperties) {
    //     if (strcmp(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME, prop.extensionName) == 0) {
    //         foundExtension = true;
    //         break;
    //     }

    // }

    // if (!foundExtension) {
    //     throw std::runtime_error("VK Instance does not support variable pointers\n");
    // }
    // enabledExtensions.push_back(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME);

    /*
        Next, we actually create the m_vkInstance.

        */

    /*
        Contains application info. This is actually not that important.
        The only real important field is apiVersion.
        */
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Erosion Compute";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName = "TerrainEngine";
    applicationInfo.engineVersion = 0;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;

    // Give our desired layers and extensions to vulkan.
    createInfo.enabledLayerCount = enabledLayers.size();
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    /*
        Actually create the m_vkInstance.
        Having created the m_vkInstance, we can actually start using vulkan.
        */
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &m_vkInstance));

    /*
        Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from
       the validation layer are actually printed.
        */
    if (enableValidationLayers)
    {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                           VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFn;

        // We have to explicitly load this function.
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr)
        {
            throw std::runtime_error("Could not load vkCreateDebugReportCallbackEXT");
        }

        // Create and register callback.
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(m_vkInstance, &createInfo, NULL, &debugReportCallback));
    }
}

void CGPUHydraulicErosion::CreatePhysicalDevice()
{
    /*
        In this function, we find a physical device that can be used with Vulkan.
        */

    /*
        So, first we will list all physical devices on the system with vkEnumeratePhysicalDevices .
        */
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, NULL);
    if (deviceCount == 0)
    {
        throw std::runtime_error("[ERRO] Could not find a device with vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

    /*
        Next, we choose a device that can be used for our purposes.
        With VkPhysicalDeviceFeatures(), we can retrieve a fine-grained list of physical features supported by the
       device. However, in this demo, we are simply launching a simple compute shader, and there are no special physical
       features demanded for this task. With VkPhysicalDeviceProperties(), we can obtain a list of physical device
       properties. Most importantly, we obtain a list of physical device limitations. For this application, we launch a
       compute shader, and the maximum size of the workgroups and total number of compute shader invocations is limited
       by the physical device, and we should ensure that the limitations named maxComputeWorkGroupCount,
       maxComputeWorkGroupInvocations and maxComputeWorkGroupSize are not exceeded by our application.  Moreover, we are
       using a storage buffer in the compute shader, and we should ensure that it is not larger than the device can
       handle, by checking the limitation maxStorageBufferRange. However, in our application, the workgroup size and
       total number of shader invocations is relatively small, and the storage buffer is not that large, and thus a vast
       majority of devices will be able to handle it. This can be verified by looking at some devices at_
        https://vulkan.gpuinfo.org/
        Therefore, to keep things simple and clean, we will not perform any such checks here, and just pick the first
       physical device in the list. But in a real and serious application, those limitations should certainly be taken
       into account.
        */

    auto bHasDevice = false;

    for (VkPhysicalDevice device : devices)
    {
        uint32_t nCapabilities = 0;

        // Get Capabilities
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(device, nullptr, &nCapabilities, nullptr));

        // Allocate
        std::vector<VkExtensionProperties> vkDeviceCapabilities(nCapabilities);

        VK_CHECK_RESULT(
            vkEnumerateDeviceExtensionProperties(device, nullptr, &nCapabilities, vkDeviceCapabilities.data()));

        for (VkExtensionProperties cap : vkDeviceCapabilities)
        {
            if (strcmp(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME, cap.extensionName) == 0)
            {
                m_vkPhyDevice = device;
                bHasDevice = true;
                break;
            }
        }
    }

    if (!bHasDevice)
    {
        throw std::runtime_error("No Vulkan device supports Variable Pointers\n");
    }
}

void CGPUHydraulicErosion::CreateDevice()
{
    /*
        We create the logical device in this function.
        */

    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhyDevice, &queueFamilyCount, NULL);

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhyDevice, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports compute.
    uint32_t queueIndex = 0;
    for (; queueIndex < queueFamilies.size(); ++queueIndex)
    {
        VkQueueFamilyProperties props = queueFamilies[queueIndex];

        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT))
        {
            // found a queue with compute. We're done!
            break;
        }
    }

    if (queueIndex == queueFamilies.size())
    {
        throw std::runtime_error("could not find a queue family that supports operations");
    }

    /*
        When creating the device, we also specify what queues it has.
        */
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = queueIndex;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.
    float queuePriorities = 1.0;    // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    /*
        Now we create the logical device. The logical device allows us to interact with the physical
        device.
        */
    VkDeviceCreateInfo deviceCreateInfo = {};

    // Specify any desired device features here. We do not need any for this application, though.
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size(); // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos =
        &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK_RESULT(vkCreateDevice(m_vkPhyDevice, &deviceCreateInfo, NULL, &m_vkDevice)); // create logical device.

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndex, 0, &m_vkQueue);
}

void CGPUHydraulicErosion::CreateBuffer(FDeviceBackedBuffer &stBuffer, uint64_t uSize, uint64_t wantedFlags,
                                        EGPUBufferTypes eBufferType)
{
    uint64_t usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    switch (eBufferType)
    {
    case EGPUBufferTypes::GPU_ONLY:
        break; // No extra flags
    case EGPUBufferTypes::COPY_DEST:
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        break;
    case EGPUBufferTypes::COPY_SOURCE:
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        break;
    case EGPUBufferTypes::COPY_BOTH:
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        break;
    default:
        break;
    };

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = uSize;                            // buffer size in bytes.
    bufferCreateInfo.usage = usageFlags;                      // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time.

    VK_CHECK_RESULT(vkCreateBuffer(m_vkDevice, &bufferCreateInfo, NULL, &stBuffer.vkBuffer)); // create buffer.
    /*
        But the buffer doesn't allocate memory for itself, so we must do that manually.
        */
    /*
        First, we find the memory requirements for the buffer.
        */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_vkDevice, stBuffer.vkBuffer, &memoryRequirements);

    /*
        Now use obtained memory requirements info to allocate the memory for the buffer.
        */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size; // specify required memory.
    /*
        There are several types of memory that can be allocated, and we must choose a memory type that:
        1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits).
        2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
                with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
        Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily
        visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
        this flag. */

    uint32_t memType = UINT32_MAX;
    // uint64_t wantedFlags = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhyDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & wantedFlags) == wantedFlags))
        {
            memType = i;
        }
    }

    if (memType == UINT32_MAX)
    {
        throw std::exception();
    }

    allocateInfo.memoryTypeIndex = memType;

    VK_CHECK_RESULT(
        vkAllocateMemory(m_vkDevice, &allocateInfo, NULL, &stBuffer.vkDeviceMemory)); // allocate memory on device.

    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory.
    VK_CHECK_RESULT(vkBindBufferMemory(m_vkDevice, stBuffer.vkBuffer, stBuffer.vkDeviceMemory, 0));

    stBuffer.uSize = uSize;
}

void CGPUHydraulicErosion::CreateBuffer(FDeviceBackedBuffer &stBuffer, uint64_t uSize, EGPUBufferTypes eBufferType)
{
    uint64_t wantedFlags = (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); //  | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                  //  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    CreateBuffer(stBuffer, uSize, wantedFlags, eBufferType);
}

void CGPUHydraulicErosion::CreateShaderModules(std::filesystem::path shaderPath)
{
    std::ifstream shaderLoader(shaderPath, std::ios::binary | std::ios::ate);

    if (!shaderLoader)
    {
        // Houston, we have a problem!
        printf("ERROR: No such file! %s\n", shaderPath.c_str());
        throw std::exception();
    }

    auto fSize = shaderLoader.tellg();
    shaderLoader.seekg(std::ios::beg);

    // Check Size
    if (fSize % sizeof(uint32_t) != 0)
    {
        printf("ERROR: Shader %s is not the correct size\n", shaderPath.c_str());
        throw std::exception();
    }

    auto bytecodeLength = fSize / sizeof(uint32_t);

    m_vShaders.emplace_back();
    auto shader = &m_vShaders[m_vShaders.size() - 1];

    shader->vBytecode.resize(bytecodeLength);

    // Load SPV to Bytecode vector
    shaderLoader.read(reinterpret_cast<char *>(shader->vBytecode.data()), fSize);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, 0, 0,
                                                       shader->vBytecode.size() * sizeof(uint32_t),
                                                       shader->vBytecode.data()};

    VK_CHECK_RESULT(vkCreateShaderModule(m_vkDevice, &shaderModuleCreateInfo, 0, &(shader->vkShaderModule)));

    // Assert
    // printf("m_vShader %ld\n", m_vShaders[m_vShaders.size() - 1].vBytecode.size());
}

void CGPUHydraulicErosion::CreateDescSetLayout()
{
    // Create the common Descriptor Set for this codebase
    VkDescriptorSetLayoutBinding descLayout[9] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {7, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
        {8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0}};

    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, 9,
                                                      descLayout};

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_vkDevice, &descLayoutInfo, 0, &m_vkDescriptorSetLayout));
}

void CGPUHydraulicErosion::CreateComputePipe()
{
    // Create 3 pipes
    m_vPipelines.resize(m_vShaders.size());

    for (uint32_t i = 0; i < m_vShaders.size(); ++i)
    {
        auto currentPipeline = &m_vPipelines[i];

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, 0, 0, 1, &m_vkDescriptorSetLayout, 0, 0};
        VK_CHECK_RESULT(
            vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, 0, &(currentPipeline->vkPipelineLayout)));

        VkComputePipelineCreateInfo computePipelineCreateInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                                                 0,
                                                                 0,
                                                                 {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                                  0, 0, VK_SHADER_STAGE_COMPUTE_BIT,
                                                                  m_vShaders[i].vkShaderModule, "main", 0},
                                                                 currentPipeline->vkPipelineLayout,
                                                                 0,
                                                                 0};

        VK_CHECK_RESULT(
            vkCreateComputePipelines(m_vkDevice, 0, 1, &computePipelineCreateInfo, 0, &(currentPipeline->vkPipeline)));
    }
}

void CGPUHydraulicErosion::CreateDescSet()
{
    VkDescriptorPoolSize descriptorPoolSize = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9};

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 0, 0, 1, 1, &descriptorPoolSize};

    VK_CHECK_RESULT(vkCreateDescriptorPool(m_vkDevice, &descriptorPoolCreateInfo, 0, &m_vkDescriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 0,
                                                             m_vkDescriptorPool, 1, &m_vkDescriptorSetLayout};

    VK_CHECK_RESULT(vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocateInfo, &m_vkDescriptorSet));
}

void CGPUHydraulicErosion::CreateCommandBuffer()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 0,
                                                     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamilyIndex};

    VK_CHECK_RESULT(vkCreateCommandPool(m_vkDevice, &commandPoolCreateInfo, 0, &m_vkCommandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 0,
                                                             m_vkCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};

    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, &m_vkCommandBuffer));
}

void CGPUHydraulicErosion::DeleteBuffer(FDeviceBackedBuffer &stBuffer)
{
    vkFreeMemory(m_vkDevice, stBuffer.vkDeviceMemory, NULL);
    vkDestroyBuffer(m_vkDevice, stBuffer.vkBuffer, NULL);
}

void CGPUHydraulicErosion::VkMemcpy(void *cpuBuffer, FDeviceBackedBuffer &stBuffer, uint64_t uSize)
{
    // Stage
    FDeviceBackedBuffer StageBuffer{};
    CreateBuffer(StageBuffer, stBuffer.uSize,
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                 EGPUBufferTypes::COPY_DEST);

    VkBufferCopy stageCopy{0, 0, stBuffer.uSize};

    // Command for Buffer Copy
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
                                                       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0};

    VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipeline);
    vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipelineLayout, 0, 1,
                            &m_vkDescriptorSet, 0, 0);
    vkCmdCopyBuffer(m_vkCommandBuffer, stBuffer.vkBuffer, StageBuffer.vkBuffer, 1, &stageCopy);

    VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, 0, 0, 0, 0, 1, &m_vkCommandBuffer, 0, 0};
    VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
    VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
    VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));

    void *hostPtr;
    VK_CHECK_RESULT(vkMapMemory(m_vkDevice, StageBuffer.vkDeviceMemory, 0, StageBuffer.uSize, 0, &hostPtr));

    memcpy(cpuBuffer, hostPtr, std::min(StageBuffer.uSize, uSize));

    vkUnmapMemory(m_vkDevice, StageBuffer.vkDeviceMemory);
    DeleteBuffer(StageBuffer);
}

void CGPUHydraulicErosion::VkMemcpy(FDeviceBackedBuffer &stBuffer, void *cpuBuffer, uint64_t uSize)
{
    // Stage
    FDeviceBackedBuffer StageBuffer{};
    CreateBuffer(StageBuffer, stBuffer.uSize,
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                 EGPUBufferTypes::COPY_SOURCE);

    void *hostPtr;
    VK_CHECK_RESULT(vkMapMemory(m_vkDevice, StageBuffer.vkDeviceMemory, 0, StageBuffer.uSize, 0, &hostPtr));

    memcpy(hostPtr, cpuBuffer, std::min(StageBuffer.uSize, uSize));

    vkUnmapMemory(m_vkDevice, StageBuffer.vkDeviceMemory);

    VkBufferCopy stageCopy{0, 0, stBuffer.uSize};

    // Command for Buffer Copy
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
                                                       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0};

    VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipeline);
    vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipelineLayout, 0, 1,
                            &m_vkDescriptorSet, 0, 0);
    vkCmdCopyBuffer(m_vkCommandBuffer, StageBuffer.vkBuffer, stBuffer.vkBuffer, 1, &stageCopy);

    VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, 0, 0, 0, 0, 1, &m_vkCommandBuffer, 0, 0};
    VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
    VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
    VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));

    DeleteBuffer(StageBuffer);
}

void CGPUHydraulicErosion::VkZeroMemory(FDeviceBackedBuffer &stBuffer)
{
    // Stage
    FDeviceBackedBuffer StageBuffer{};
    CreateBuffer(StageBuffer, stBuffer.uSize,
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
                 EGPUBufferTypes::COPY_SOURCE);

    void *hostPtr;
    VK_CHECK_RESULT(vkMapMemory(m_vkDevice, StageBuffer.vkDeviceMemory, 0, StageBuffer.uSize, 0, &hostPtr));

    memset(hostPtr, 0, stBuffer.uSize);

    vkUnmapMemory(m_vkDevice, StageBuffer.vkDeviceMemory);

    VkBufferCopy stageCopy{0, 0, stBuffer.uSize};

    // Command for Buffer Copy
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
                                                       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0};

    VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipeline);
    vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[0].vkPipelineLayout, 0, 1,
                            &m_vkDescriptorSet, 0, 0);
    vkCmdCopyBuffer(m_vkCommandBuffer, StageBuffer.vkBuffer, stBuffer.vkBuffer, 1, &stageCopy);

    VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, 0, 0, 0, 0, 1, &m_vkCommandBuffer, 0, 0};
    VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
    VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
    VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));

    DeleteBuffer(StageBuffer);
}

void CGPUHydraulicErosion::UpdateDescriptorSets(uint32_t uDescCount, FDeviceBackedBuffer **ppElements)
{
    std::vector<VkDescriptorBufferInfo> bufInfos;
    bufInfos.resize(uDescCount);

    for (uint32_t i = 0; i < uDescCount; ++i)
    {
        bufInfos[i] = VkDescriptorBufferInfo{ppElements[i]->vkBuffer, 0, VK_WHOLE_SIZE};
    }

    // VkDescriptorBufferInfo hmapDescriptorBufferInfo = {
    // 	heightMap.vkBuffer,
    // 	0,
    // 	VK_WHOLE_SIZE};

    std::vector<VkWriteDescriptorSet> descInfos;
    descInfos.resize(uDescCount);

    for (uint32_t i = 0; i < uDescCount; ++i)
    {
        descInfos[i] = VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 0, m_vkDescriptorSet, i, 0, 1,
                                            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,      0, &bufInfos[i],      0};
    }

    // VkWriteDescriptorSet writeDescriptorSet[8] = {
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 0,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &hmapDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 2,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &waterMapDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 3,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &sedimentMapDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 4,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &waterLevelDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 5,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &sedimentLevelDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 6,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &delayedWaterLevelDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 7,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &delayedSedimentLevelDescriptorBufferInfo,
    // 	 0},
    // 	{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    // 	 0,
    // 	 m_vkDescriptorSet,
    // 	 8,
    // 	 0,
    // 	 1,
    // 	 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    // 	 0,
    // 	 &invokeInfoDescriptorBufferInfo,
    // 	 0}};

    vkUpdateDescriptorSets(m_vkDevice, uDescCount, descInfos.data(), 0, 0);
}

// This function may a monolith
void CGPUHydraulicErosion::Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth,
                                 uint32_t uSteps, FLOAT_TYPE *pTerrain, float fScale)
{
    // Create In and Out Buffers
    FDeviceBackedBuffer heightMap{};
    FDeviceBackedBuffer terrainMap{};
    FDeviceBackedBuffer waterMap{};
    FDeviceBackedBuffer sedimentMap{};
    FDeviceBackedBuffer waterLevel{};
    FDeviceBackedBuffer sedimentLevel{};
    FDeviceBackedBuffer delayedWaterLevel{};
    FDeviceBackedBuffer delayedSedimentLevel{};
    FDeviceBackedBuffer invokeInfo{};

    // Create Heightmap and copy!
    FShaderInfo cpuSideInfo{};
    cpuSideInfo.uWidth = uWidth;
    cpuSideInfo.uHeight = uHeight;
    cpuSideInfo.fSoilSoftness = 0.3f;
    cpuSideInfo.fSedimentCapacity = 1.f;
    cpuSideInfo.fEvaporationRate = 0.01f;
    cpuSideInfo.fDepositionRate = 0.3f / 10;
    cpuSideInfo.fRainCoeff = 0.0125f;
    cpuSideInfo.fWaterLevel = m_fWaterLevel;
    cpuSideInfo.bClearTerrain = false;

    CreateBuffer(heightMap, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::COPY_BOTH);
    VkMemcpy(heightMap, pHeight, uWidth * uHeight * sizeof(FLOAT_TYPE));

    if (pTerrain)
    {
        CreateBuffer(terrainMap, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::COPY_DEST);
        VkMemcpy(terrainMap, pTerrain, uWidth * uHeight * sizeof(FLOAT_TYPE));
    }
    else
    {
        CreateBuffer(terrainMap, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::GPU_ONLY);
        cpuSideInfo.bClearTerrain = true;
    }

    CreateBuffer(invokeInfo, sizeof(FShaderInfo), EGPUBufferTypes::COPY_DEST);
    VkMemcpy(invokeInfo, &cpuSideInfo, sizeof(FShaderInfo));

    CreateBuffer(waterMap, uWidth * uHeight * sizeof(FLOAT_TYPE) * 8, EGPUBufferTypes::GPU_ONLY);
    CreateBuffer(sedimentMap, uWidth * uHeight * sizeof(FLOAT_TYPE) * 8, EGPUBufferTypes::GPU_ONLY);
    CreateBuffer(waterLevel, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::GPU_ONLY);
    CreateBuffer(sedimentLevel, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::GPU_ONLY);
    CreateBuffer(delayedWaterLevel, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::GPU_ONLY);
    CreateBuffer(delayedSedimentLevel, uWidth * uHeight * sizeof(FLOAT_TYPE), EGPUBufferTypes::GPU_ONLY);

    // printf("Zeroing Arrays... ");
    // VkZeroMemory(waterMap);
    // VkZeroMemory(sedimentMap);
    // VkZeroMemory(waterLevel);
    // VkZeroMemory(sedimentLevel);
    // VkZeroMemory(delayedWaterLevel);
    // VkZeroMemory(delayedSedimentLevel);
    // printf("\rZeroing Arrays. Done.\n");

    // Desc Pools
    FDeviceBackedBuffer *bufList[9] = {&heightMap,         &terrainMap,           &waterMap,
                                       &sedimentMap,       &waterLevel,           &sedimentLevel,
                                       &delayedWaterLevel, &delayedSedimentLevel, &invokeInfo};

    UpdateDescriptorSets(9, bufList);

    uint32_t safeHeight = ((uHeight - 1) / 32) + 1;
    uint32_t safeWidth = ((uWidth - 1) / 32) + 1;

    // commandBuffer info
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 0,
                                                       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0};

    // Zero GPU arrays on the GPU to avoid memory use
    VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[4].vkPipeline);
    vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[4].vkPipelineLayout, 0, 1,
                            &m_vkDescriptorSet, 0, 0);
    vkCmdDispatch(m_vkCommandBuffer, safeWidth, safeHeight, 1);
    VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, 0, 0, 0, 0, 1, &m_vkCommandBuffer, 0, 0};

    VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
    VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
    VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));

    // // Process Rain Once :)
    VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[2].vkPipeline);
    vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[2].vkPipelineLayout, 0, 1,
                            &m_vkDescriptorSet, 0, 0);
    vkCmdDispatch(m_vkCommandBuffer, safeWidth, safeHeight, 1);
    VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));

    VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
    VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
    VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));

    // uint32_t uSteps = 1500;

    for (uint32_t s = 0; s < uSteps; ++s)
    {
        if (s % 100 == 0)
        {
            printf("\rStarting iteration %d (%f%%)", s, float(s * 100) / uSteps);
            fflush(stdout);
        }

        // Shader Limit
        uint32_t shaderRain = (s % 500 == 0) && (s < uSteps * 0.9);
        uint32_t shaderDump = (s + 1 == uSteps);

        // Apply Both
        for (uint32_t i = 0; i < 2 + shaderRain + shaderDump; ++i)
        // uint32_t i = 0;
        {
            // printf("Applying Shader %d\n", i);
            // Process :)
            // VkCommandBufferBeginInfo commandBufferBeginInfo = {
            // 	VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            // 	0,
            // 	VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            // 	0};

            VK_CHECK_RESULT(vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo));

            vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[i].vkPipeline);

            vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vPipelines[i].vkPipelineLayout,
                                    0, 1, &m_vkDescriptorSet, 0, 0);

            // This may be changed for speed!
            vkCmdDispatch(m_vkCommandBuffer, safeWidth, safeHeight, 1);
            // vkCmdDispatch(m_vkCommandBuffer, uWidth, uHeight, 1);

            VK_CHECK_RESULT(vkEndCommandBuffer(m_vkCommandBuffer));

            // VkSubmitInfo submitInfo =
            // 	{
            // 		VK_STRUCTURE_TYPE_SUBMIT_INFO,
            // 		0,
            // 		0,
            // 		0,
            // 		0,
            // 		1,
            // 		&m_vkCommandBuffer,
            // 		0,
            // 		0
            // 	};

            VK_CHECK_RESULT(vkQueueSubmit(m_vkQueue, 1, &submitInfo, 0));
            VK_CHECK_RESULT(vkQueueWaitIdle(m_vkQueue));
            VK_CHECK_RESULT(vkResetCommandBuffer(m_vkCommandBuffer, 0));
        }
    }

    printf("Device to Host Copy...");
    VkMemcpy(pOut, heightMap, uWidth * uHeight * sizeof(FLOAT_TYPE));
    printf("\rDevice to Host Copy. Done.\n\n");

    DeleteBuffer(heightMap);
    DeleteBuffer(waterMap);
    DeleteBuffer(sedimentMap);
    DeleteBuffer(waterLevel);
    DeleteBuffer(sedimentLevel);
    DeleteBuffer(delayedWaterLevel);
    DeleteBuffer(delayedSedimentLevel);

    // CHydraulicErosion::Erode(pHeight, pOut, uHeight, uWidth, fScale);
}

void CGPUHydraulicErosion::ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth,
                                          float fScale)
{
    CHydraulicErosion::ErodeByNormals(pHeight, pOut, uHeight, uWidth, fScale);
}

#endif