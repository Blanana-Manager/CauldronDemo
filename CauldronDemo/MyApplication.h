#pragma once

#include <Windows.h>
#include <iostream>
#include "Misc/FrameworkWindows.h"
#include "DjpDevice.h"

#include <vector>
#include "DjpSwapChain.h"

class MyApplication : public FrameworkWindows
{
private:
    CAULDRON_VK::DjpDevice* m_device = new CAULDRON_VK::DjpDevice();
    CAULDRON_VK::DjpSwapChain* m_swapChain = new CAULDRON_VK::DjpSwapChain();
    
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    int m_currentCommandIndex = 0;

    VkSemaphore* p_imageAvailableSemaphore = new VkSemaphore();
    VkSemaphore* p_renderFinishSemaphore = new VkSemaphore();
    VkFence* p_cmdBufExecutedFences = new VkFence();

public:
	MyApplication():FrameworkWindows("Culdron Vulkan"){}
    MyApplication(uint32_t width,uint32_t height) :FrameworkWindows("Culdron Vulkan")
    {
        m_Width = width;
        m_Height = height;
    }

    virtual void OnCreate(HWND hWnd);
    virtual void OnDestroy();
    virtual void OnRender();
    virtual bool OnEvent(MSG msg);
    virtual void OnResize(uint32_t Width, uint32_t Height);
    virtual void SetFullScreen(bool fullscreen);
};
