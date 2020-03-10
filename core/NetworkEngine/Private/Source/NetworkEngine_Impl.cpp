#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"

CNetworkEngine_Impl::CNetworkEngine_Impl()
: bIsListening(false)
{
    m_trNetHandler = std::thread(&CNetworkEngine_Impl::NetworkMain_RT, this);
}

CNetworkEngine_Impl::~CNetworkEngine_Impl()
{

}

ENetworkErrorCodes CNetworkEngine_Impl::RegisterNode
(
    std::string strHostname, 
    uint16_t uPort,
    std::vector<char> vKey
)
{
    return ENetworkErrorCodes::TOTAL_ERROR_CODES;
}
 
ENetworkErrorCodes CNetworkEngine_Impl::RegisterNode
(
    std::string strHostname, 
    uint16_t uPort
)
{
    return ENetworkErrorCodes::TOTAL_ERROR_CODES;
}

ENetworkErrorCodes CNetworkEngine_Impl::Listen(uint16_t uPort)
{
    // Start a listening thread
    m_trListener = std::thread(&CNetworkEngine_Impl::Listener_RT, this);


    
}

        // std::unique_lock<std::mutex> lock(m_connectionQueueMutex);

        // // Queue not empty
        // m_cond_queue.wait(lock, [this](){ return !m_connectionQueue.empty(); });

        //         // Readd to queue
        // std::unique_lock<std::mutex> secondlock(m_connectionQueueMutex);

        // // Queue not empty
        // //m_cond_queue.wait(lock, [this](){ !m_connectionQueue.empty() });

        // // Push to back
        // m_connectionQueue.push(pTask);

        // // Unlock Mutex
        // secondlock.unlock();

        // // Notify another
        // m_cond_queue.notify_one();