#include "MyApplication.h"
#include "base/Device.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL info_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void MyApplication::OnCreate(HWND hWnd)
{
    m_device->OnCreate("Cauldron Vk", "Cauldron", true, hWnd);
 
    m_swapChain->OnCreate(reinterpret_cast<CAULDRON_VK::Device*>(m_device), 3, hWnd, CAULDRON_VK::DisplayModes::DISPLAYMODE_SDR);
    m_swapChain->OnCreateWindowSizeDependentResources(m_Width, m_Height,true, CAULDRON_VK::DisplayModes::DISPLAYMODE_SDR);
    
    VkCommandPoolCreateInfo cpci = {};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = m_device->GetGraphicsQueueFamilyIndex();
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    vkCreateCommandPool(m_device->GetDevice(), &cpci, nullptr, &m_commandPool);

    uint32_t commandBufferCount = 3;
    m_commandBuffers.resize(commandBufferCount);
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_commandPool;
    //指定 缓冲对象为 主缓冲
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = commandBufferCount;

    vkAllocateCommandBuffers(m_device->GetDevice(), &allocateInfo, m_commandBuffers.data());
}

void MyApplication::OnDestroy()
{
}

void MyApplication::OnRender()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkExtent2D extent{ m_Width,m_Height };

    VkRenderPassBeginInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderInfo.renderPass = m_swapChain->GetRenderPass();
    renderInfo.renderArea.offset = { 0,0 };
    renderInfo.renderArea.extent = extent;
    VkClearValue clearValue[2];
    clearValue[0].color = { 0.2f,0.5f,0.0f,0.0f };
    clearValue[1].depthStencil = { 1.0f,0 };
    renderInfo.clearValueCount = 2;
    renderInfo.pClearValues = clearValue;

    //for (int i = 0; i < m_commandBuffers.size(); ++i)
    //{
    //    vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);

    //    renderInfo.framebuffer = m_swapChain->GetFramebuffer(i);

    //    vkCmdBeginRenderPass(m_commandBuffers[i], &renderInfo, VK_SUBPASS_CONTENTS_INLINE);

    //    vkCmdEndRenderPass(m_commandBuffers[i]);

    //    vkEndCommandBuffer(m_commandBuffers[i]);//于 beginCommandBuffer 对应

    //}

    vkBeginCommandBuffer(m_commandBuffers[m_currentCommandIndex], &beginInfo);

    renderInfo.framebuffer = m_swapChain->GetFramebuffer(m_currentCommandIndex);

    vkCmdBeginRenderPass(m_commandBuffers[m_currentCommandIndex], &renderInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdEndRenderPass(m_commandBuffers[m_currentCommandIndex]);

    vkEndCommandBuffer(m_commandBuffers[m_currentCommandIndex]);//于 beginCommandBuffer 对应

    int index = m_swapChain->WaitForSwapChain();

    VkSubmitInfo submitInfo{};
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    m_swapChain->GetSemaphores(p_imageAvailableSemaphore, p_renderFinishSemaphore, p_cmdBufExecutedFences);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentCommandIndex];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = p_imageAvailableSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = p_renderFinishSemaphore;
    submitInfo.pWaitDstStageMask = &flags;

    vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, *p_cmdBufExecutedFences);

    m_swapChain->Present();
    vkWaitForFences(m_device->GetDevice(), 1, p_cmdBufExecutedFences, VK_TRUE, UINT64_MAX);

    m_currentCommandIndex = (m_currentCommandIndex + 1) % m_commandBuffers.size();
}

bool MyApplication::OnEvent(MSG msg)
{
    return false;
}

void MyApplication::OnResize(uint32_t Width, uint32_t Height)
{
}

void MyApplication::SetFullScreen(bool fullscreen)
{
}
