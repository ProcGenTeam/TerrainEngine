#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"
#include "NetworkEngine/Public/Header/ErrorCode.h"

CNetworkEngine_Impl::CNetworkEngine_Impl()
{
    //m_trNetHandler = std::thread(&CNetworkEngine_Impl::NetworkMain_RT, this);
}

CNetworkEngine_Impl::~CNetworkEngine_Impl()
{

}

ENetworkErrorCodes CNetworkEngine_Impl::SubmitTask(FTask &task, std::vector<FOperation> &vHistory)
{
    return ENetworkErrorCodes::TOTAL_ERROR_CODES;
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
    // Check port
    if(uPort == 0) { return ENetworkErrorCodes::Listen_InvalidPort; }

    // Try accept the port
    BIO *ListenSocket = BIO_new_accept(std::to_string(uPort).c_str());

    if(ListenSocket)
    {
        auto res = BIO_do_accept(ListenSocket);

        if(res <= 0)
        {
            return ENetworkErrorCodes::Listen_AcceptError;
        }

        // Handle
        //m_bIsListening = true;
    }
    else
    {
        return ENetworkErrorCodes::Listen_CreateError;
    }

    // Start a listening thread
    //m_trListener = std::thread(&CNetworkEngine_Impl::Listener_RT, this);

    // Get ready to handle task dispatching
    //m_trDispatch = std::thread(&CNetworkEngine_Impl::Dispatch_RT, this);

    // Fallen Through
    return ENetworkErrorCodes::NoError;
}

void CNetworkEngine_Impl::SubmitMessage(FNetMessage &&Message)
{
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    // We have no wait condition :)
    m_qMessageQueue.emplace(std::move(Message));

    // Unlock and notify
    lock.unlock();
    m_cvMessageQueue.notify_one();
}

FNetMessage CNetworkEngine_Impl::ReadMessage()
{
    // Wait on not being empty
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    m_cvMessageQueue.wait(lock, [this](){
        return !m_qMessageQueue.empty();
    });

    // Get Data
    auto retVar = m_qMessageQueue.front();
    m_qMessageQueue.pop();

    // Unlock.
    // Not strictly necessary are unique_lock is scoped
    lock.unlock();

    return retVar;
}

void CNetworkEngine_Impl::Listener_RT()
{

}

void CNetworkEngine_Impl::Dispatch_RT()
{

}

        // std::unique_lock<std::mutex> lock(m_connectionQueueMutex);

        // // Queue not empty
        // m_cond_queue.wait(lock, [this](){ return !m_connectionQueue.empty(); });

       
       
       
       
        // // Readd to queue
        // std::unique_lock<std::mutex> secondlock(m_connectionQueueMutex);

        // // Queue not empty
        // //m_cond_queue.wait(lock, [this](){ !m_connectionQueue.empty() });

        // // Push to back
        // m_connectionQueue.push(pTask);

        // // Unlock Mutex
        // secondlock.unlock();

        // // Notify another
        // m_cond_queue.notify_one();