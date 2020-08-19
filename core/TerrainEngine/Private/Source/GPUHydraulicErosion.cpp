#ifdef TE_USE_GPU
#include "core/TerrainEngine/Private/Header/GPUHydraulicErosion.h"
#include <stdexcept>
#include <cstring>
#include <cassert>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__);                  \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage,
    void*                                       pUserData) 
{
    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);
    return VK_FALSE;
}

CGPUHydraulicErosion::CGPUHydraulicErosion(int32_t iOverscan, uint32_t uSeed, int32_t iOffsetX, int32_t iOffsetY, FLOAT_TYPE fWaterLevel)
: CHydraulicErosion(iOverscan, uSeed, iOffsetX, iOffsetY, fWaterLevel)
{
    printf("GoingForCreate\n");
    CreateVKInstance();
    CreatePhysicalDevice();
    CreateDevice();
    //CreateBuffer(m_heightMapBuffer, m_heightMapBufferMemory, );
    CreateDescSetLayout();
    CreateDescSet();
    CreateComputePipe();
    CreateCommandBuffer();
}

CGPUHydraulicErosion::~CGPUHydraulicErosion()
{
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
        for (VkLayerProperties prop : layerProperties) {
            
            if (strcmp("VK_LAYER_LUNARG_standard_validation", prop.layerName) == 0) {
                foundLayer = true;
                break;
            }

        }
        
        if (!foundLayer) {
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
        for (VkExtensionProperties prop : extensionProperties) {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
                foundExtension = true;
                break;
            }

        }

        if (!foundExtension) {
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
    VK_CHECK_RESULT(vkCreateInstance(
        &createInfo,
        NULL,
        &m_vkInstance));

    /*
    Register a callback function for the extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the validation
    layer are actually printed.
    */
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        createInfo.pfnCallback = &debugReportCallbackFn;

        // We have to explicitly load this function.
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT == nullptr) {
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
    With VkPhysicalDeviceFeatures(), we can retrieve a fine-grained list of physical features supported by the device.
    However, in this demo, we are simply launching a simple compute shader, and there are no 
    special physical features demanded for this task.
    With VkPhysicalDeviceProperties(), we can obtain a list of physical device properties. Most importantly,
    we obtain a list of physical device limitations. For this application, we launch a compute shader,
    and the maximum size of the workgroups and total number of compute shader invocations is limited by the physical device,
    and we should ensure that the limitations named maxComputeWorkGroupCount, maxComputeWorkGroupInvocations and 
    maxComputeWorkGroupSize are not exceeded by our application.  Moreover, we are using a storage buffer in the compute shader,
    and we should ensure that it is not larger than the device can handle, by checking the limitation maxStorageBufferRange. 
    However, in our application, the workgroup size and total number of shader invocations is relatively small, and the storage buffer is
    not that large, and thus a vast majority of devices will be able to handle it. This can be verified by looking at some devices at_
    https://vulkan.gpuinfo.org/
    Therefore, to keep things simple and clean, we will not perform any such checks here, and just pick the first physical
    device in the list. But in a real and serious application, those limitations should certainly be taken into account.
    */

    auto bHasDevice = false;

    for (VkPhysicalDevice device : devices) 
    {
        uint32_t nCapabilities = 0;

        // Get Capabilities
        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &nCapabilities,
                nullptr
        ));

        // Allocate
        std::vector<VkExtensionProperties> vkDeviceCapabilities(nCapabilities);

        VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &nCapabilities,
                vkDeviceCapabilities.data()
        ));

        for (VkExtensionProperties cap : vkDeviceCapabilities) 
        {
            if (strcmp(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME, cap.extensionName) == 0) {
                m_vkPhyDevice = device;
                bHasDevice = true;
                break;
            }
        }
    }

    if(!bHasDevice)
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
    for (; queueIndex < queueFamilies.size(); ++queueIndex) {
        VkQueueFamilyProperties props = queueFamilies[queueIndex];

        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            // found a queue with compute. We're done!
            break;
        }
    }

    if (queueIndex == queueFamilies.size()) {
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
    float queuePriorities = 1.0;  // we only have one queue, so this is not that imporant. 
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    /*
    Now we create the logical device. The logical device allows us to interact with the physical
    device.
    */
    VkDeviceCreateInfo deviceCreateInfo = {};

    // Specify any desired device features here. We do not need any for this application, though.
    VkPhysicalDeviceFeatures deviceFeatures = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = enabledLayers.size();  // need to specify validation layers here as well.
    deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // when creating the logical device, we also specify what queues it has.
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VK_CHECK_RESULT(vkCreateDevice(m_vkPhyDevice, &deviceCreateInfo, NULL, &m_vkDevice)); // create logical device.

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(m_vkDevice, queueFamilyIndex, 0, &queue);
}

void CGPUHydraulicErosion::CreateBuffer(FDeviceBackedBuffer &stBuffer, uint64_t uSize)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = uSize; // buffer size in bytes. 
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
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

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vkPhyDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if (
            (memoryRequirements.memoryTypeBits & (1 << i))
             &&
            ((memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        )
        {
            memType = i;
        }
    }

    allocateInfo.memoryTypeIndex = memType;

    VK_CHECK_RESULT(vkAllocateMemory(m_vkDevice, &allocateInfo, NULL, &stBuffer.vkDeviceMemory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory. 
    VK_CHECK_RESULT(vkBindBufferMemory(m_vkDevice, stBuffer.vkBuffer, stBuffer.vkDeviceMemory, 0));
}

void CGPUHydraulicErosion::CreateDescSetLayout()
{

}

void CGPUHydraulicErosion::CreateDescSet()
{

}

void CGPUHydraulicErosion::CreateComputePipe()
{

}

void CGPUHydraulicErosion::CreateCommandBuffer()
{

}

void CGPUHydraulicErosion::DeleteBuffer(FDeviceBackedBuffer &stBuffer)
{
    vkFreeMemory(m_vkDevice, stBuffer.vkDeviceMemory, NULL);
    vkDestroyBuffer(m_vkDevice, stBuffer.vkBuffer, NULL);	
}

// This function may a monolith
void CGPUHydraulicErosion::Erode(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    // Create In and Out Buffers
    FDeviceBackedBuffer waterMap{};
    FDeviceBackedBuffer sedimentMap{};
    FDeviceBackedBuffer heightMap{};
    FDeviceBackedBuffer accumulationMap{};
    FDeviceBackedBuffer outMap{};
    
    CreateBuffer(heightMap, uWidth * uHeight * sizeof(FLOAT_TYPE));
    CreateBuffer(outMap, uWidth * uHeight * sizeof(FLOAT_TYPE));
    CreateBuffer(sedimentMap, uWidth * uHeight * sizeof(FLOAT_TYPE) * 8);
    CreateBuffer(waterMap, uWidth * uHeight * sizeof(FLOAT_TYPE) * 8);
    CreateBuffer(accumulationMap, uWidth * uHeight * sizeof(glm::vec4));

    // Create Shader
    

    // Process :)

    DeleteBuffer(accumulationMap);
    DeleteBuffer(waterMap);
    DeleteBuffer(sedimentMap);
    DeleteBuffer(outMap);
    DeleteBuffer(heightMap);

    CHydraulicErosion::Erode(pHeight, pOut, uHeight, uWidth, fScale);

}

void CGPUHydraulicErosion::ErodeByNormals(FLOAT_TYPE *pHeight, FLOAT_TYPE *pOut, uint32_t uHeight, uint32_t uWidth, float fScale)
{
    CHydraulicErosion::ErodeByNormals(pHeight, pOut, uHeight, uWidth, fScale);
}

#endif