// AMD AMDUtils code
// 
// Copyright(c) 2018 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "stdafx.h"

#include "DjpDevice.h"
#include <vulkan/vulkan_win32.h>
#include "base/Instance.h"
#include "base/InstanceProperties.h"
#include "base/DeviceProperties.h"
#include "base/ExtDebugMarkers.h"
#include "base/ExtFreeSync2.h"
#include "base/ExtFp16.h"
#include "base/ExtValidation.h"

#ifdef USE_VMA
#define VMA_IMPLEMENTATION
#include "../VulkanMemoryAllocator/vk_mem_alloc.h"
#endif

//Debug info 中的回调函数
static VKAPI_ATTR VkBool32 VKAPI_CALL info_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cout << pCallbackData->pMessage << std::endl;
    //OutputDebugString(pCallbackData->pMessage);
    return VK_FALSE;
}

static VkInstance __CreateInstance(VkApplicationInfo app_info, bool usingValidationLayer)
{
    //===========================================================
    //查询扩展
    uint32_t extension_count = 0;
    std::vector<VkExtensionProperties> extension_properties(0);
    std::vector<char*> extension_name(0);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    extension_properties.resize(extension_count);
    extension_name.resize(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_properties.data());
    for (int i = 0; i < extension_count; ++i)
    {
        extension_name[i] = extension_properties[i].extensionName;
    }
    const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    };
    //==============================================================

    VkInstance instance;

    //populate list from enabled extensions
    void* pNext = NULL;

    // read layer and instance properties
    CAULDRON_VK::InstanceProperties ip;
    ip.Init();

    // Check required extensions are present
    //
    ip.AddInstanceExtensionName(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    ip.AddInstanceExtensionName(VK_KHR_SURFACE_EXTENSION_NAME);
    ExtFreeSync2CheckInstanceExtensions(&ip);
    if (usingValidationLayer)
    {
        usingValidationLayer = ExtDebugReportCheckInstanceExtensions(&ip, &pNext);

    }

    // prepare existing extensions and layer names into a buffer for vkCreateInstance
    std::vector<const char*> instance_layer_names;
    std::vector<const char*> instance_extension_names;
    ip.GetExtensionNamesAndConfigs(&instance_layer_names, &instance_extension_names);

    // do create the instance
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = pNext;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
   /* inst_info.enabledLayerCount = (uint32_t)instance_layer_names.size();
    inst_info.ppEnabledLayerNames = (uint32_t)instance_layer_names.size() ? instance_layer_names.data() : NULL;
    inst_info.enabledExtensionCount = (uint32_t)instance_extension_names.size();
    inst_info.ppEnabledExtensionNames = instance_extension_names.data();*/
    inst_info.enabledLayerCount = validationLayers.size();
    inst_info.ppEnabledLayerNames = validationLayers.data();
    inst_info.enabledExtensionCount = extension_count;
    inst_info.ppEnabledExtensionNames = extension_name.data();

    VkResult res = vkCreateInstance(&inst_info, NULL, &instance);
    assert(res == VK_SUCCESS);

    // Init the extensions (if they have been enabled successfuly)
    //
    //
    //ExtDebugReportGetProcAddresses(instance);
    //ExtDebugReportOnCreate(instance);

    //********* 创建 debug 回调 **************************
    //--------------------------------------------------
    VkDebugUtilsMessengerCreateInfoEXT debuge_info = {};
    debuge_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debuge_info.flags = 0;
    debuge_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debuge_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debuge_info.pfnUserCallback = info_callback;
    debuge_info.pUserData = nullptr;

    static VkDebugUtilsMessengerEXT messager;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    func(instance, &debuge_info, nullptr, &messager);
    //--------------------------------------------------
    //********* 创建 debug回调 结束***********************

    return instance;
}

namespace CAULDRON_VK
{
    DjpDevice::DjpDevice()
    {
    }

    DjpDevice::~DjpDevice()
    {
    }

    void DjpDevice::OnCreate(const char *pAppName, const char *pEngineName, bool bValidationEnabled, HWND hWnd)
    {
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = NULL;
        app_info.pApplicationName = pAppName;
        app_info.applicationVersion = 1;
        app_info.pEngineName = pEngineName;
        app_info.engineVersion = 1;
        app_info.apiVersion = VK_API_VERSION_1_1;
        m_instance = __CreateInstance(app_info, bValidationEnabled);


        // Enumerate physical devices
        //
        uint32_t gpu_count = 1;
        uint32_t const req_count = gpu_count;
        VkResult res = vkEnumeratePhysicalDevices(m_instance, &gpu_count, NULL);
        assert(gpu_count);

        std::vector<VkPhysicalDevice> gpus;
        gpus.resize(gpu_count);

        res = vkEnumeratePhysicalDevices(m_instance, &gpu_count, gpus.data());
        assert(!res && gpu_count >= req_count);

        m_physicaldevice = gpus[0];

        // Get queue/memory/device properties
        //
        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicaldevice, &queue_family_count, NULL);
        assert(queue_family_count >= 1);

        std::vector<VkQueueFamilyProperties> queue_props;
        queue_props.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicaldevice, &queue_family_count, queue_props.data());
        assert(queue_family_count >= 1);

        vkGetPhysicalDeviceMemoryProperties(m_physicaldevice, &m_memoryProperties);
        vkGetPhysicalDeviceProperties(m_physicaldevice, &m_deviceProperties);

        // Crate a Win32 Surface
        //
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = NULL;
        createInfo.hinstance = NULL;
        createInfo.hwnd = hWnd;
        res = vkCreateWin32SurfaceKHR(m_instance, &createInfo, NULL, &m_surface);

        // Find a graphics device and a queue that can present to the above surface
        //
        graphics_queue_family_index = UINT32_MAX;
        present_queue_family_index = UINT32_MAX;
        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            {
                if (graphics_queue_family_index == UINT32_MAX) graphics_queue_family_index = i;

                VkBool32 supportsPresent;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, m_surface, &supportsPresent);
                if (supportsPresent == VK_TRUE)
                {
                    graphics_queue_family_index = i;
                    present_queue_family_index = i;
                    break;
                }
            }
        }

        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        if (present_queue_family_index == UINT32_MAX)
        {
            for (uint32_t i = 0; i < queue_family_count; ++i)
            {
                VkBool32 supportsPresent;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, m_surface, &supportsPresent);
                if (supportsPresent == VK_TRUE)
                {
                    present_queue_family_index = (uint32_t)i;
                    break;
                }
            }
        }

        compute_queue_family_index = UINT32_MAX;

        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if ((queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
            {
                if (compute_queue_family_index == UINT32_MAX)
                    compute_queue_family_index = i;
                if (i != graphics_queue_family_index) {
                    compute_queue_family_index = i;
                    break;
                }
            }
        }

        // Read device extension's properties 
        DeviceProperties dp;
        dp.Init(m_physicaldevice);

        // Check required extensions are present
        //
        void *pNext = NULL;
        m_usingFp16 = ExtFp16CheckExtensions(&dp, &pNext);
        ExtFreeSync2CheckDeviceExtensions(&dp);
        ExtDebugMarkerCheckDeviceExtensions(&dp);
        dp.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        dp.Add(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        dp.Add(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);

        // prepare existing extensions names into a buffer for vkCreateDevice
        std::vector<const char *> extension_names;
        dp.GetExtensionNamesAndConfigs(&extension_names);

        // Create device 
        //
        float queue_priorities[1] = { 0.0 };
        VkDeviceQueueCreateInfo queue_info[2] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].pNext = NULL;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priorities;
        queue_info[0].queueFamilyIndex = graphics_queue_family_index;
        queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[1].pNext = NULL;
        queue_info[1].queueCount = 1;
        queue_info[1].pQueuePriorities = queue_priorities;
        queue_info[1].queueFamilyIndex = compute_queue_family_index;

        VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
        physicalDeviceFeatures.fillModeNonSolid = true;
        physicalDeviceFeatures.pipelineStatisticsQuery = true;
        physicalDeviceFeatures.fragmentStoresAndAtomics = true;
        physicalDeviceFeatures.vertexPipelineStoresAndAtomics = true;
        physicalDeviceFeatures.shaderImageGatherExtended = true;
        physicalDeviceFeatures.wideLines = true; //needed for drawing lines with a specific width.

        VkDeviceCreateInfo device_info = {};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.pNext = pNext;
        device_info.queueCreateInfoCount = 2;
        device_info.pQueueCreateInfos = queue_info;
        device_info.enabledExtensionCount = (uint32_t)extension_names.size();
        device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? extension_names.data() : NULL;
        device_info.pEnabledFeatures = &physicalDeviceFeatures;
        res = vkCreateDevice(gpus[0], &device_info, NULL, &m_device);
        assert(res == VK_SUCCESS);

#ifdef USE_VMA
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = GetPhysicalDevice();
        allocatorInfo.device = GetDevice();
        vmaCreateAllocator(&allocatorInfo, &m_hAllocator);
#endif

        // create queues
        //
        vkGetDeviceQueue(m_device, graphics_queue_family_index, 0, &graphics_queue);
        if (graphics_queue_family_index == present_queue_family_index)
        {
            present_queue = graphics_queue;
        }
        else
        {
            vkGetDeviceQueue(m_device, present_queue_family_index, 0, &present_queue);
        }
        if (compute_queue_family_index != UINT32_MAX)
        {
            vkGetDeviceQueue(m_device, compute_queue_family_index, 0, &compute_queue);
        }

        // Init the extensions (if they have been enabled successfuly)
        //
        ExtDebugMarkersGetProcAddresses(m_device);
        ExtFreeSync2GetProcAddresses(m_instance, m_device);
    }

    void DjpDevice::CreatePipelineCache()
    {
        // create pipeline cache

        VkPipelineCacheCreateInfo pipelineCache;
        pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        pipelineCache.pNext = NULL;
        pipelineCache.initialDataSize = 0;
        pipelineCache.pInitialData = NULL;
        pipelineCache.flags = 0;
        VkResult res = vkCreatePipelineCache(m_device, &pipelineCache, NULL, &m_pipelineCache);
        assert(res == VK_SUCCESS);
    }

    void DjpDevice::DestroyPipelineCache()
    {
        vkDestroyPipelineCache(m_device, m_pipelineCache, NULL);
    }

    VkPipelineCache DjpDevice::GetPipelineCache()
    {
        return m_pipelineCache;
    }

    void DjpDevice::OnDestroy()
    {
        if (m_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, NULL);
        }

#ifdef USE_VMA
        vmaDestroyAllocator(m_hAllocator);
        m_hAllocator = NULL;
#endif

        if (m_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }

        DestroyInstance(m_instance);

        m_instance = VK_NULL_HANDLE;
    }

    void DjpDevice::GPUFlush()
    {
        vkDeviceWaitIdle(m_device);
    }

    bool memory_type_from_properties(VkPhysicalDeviceMemoryProperties &memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
        // Search memtypes to find first index with those properties
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
            if ((typeBits & 1) == 1) {
                // Type is available, does it match user properties?
                if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                    *typeIndex = i;
                    return true;
                }
            }
            typeBits >>= 1;
        }
        // No memory types matched, return failure
        return false;
    }
}
