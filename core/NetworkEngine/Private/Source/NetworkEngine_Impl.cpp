#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"
#include "NetworkEngine/Public/Header/ErrorCode.h"

CNetworkEngine_Impl::CNetworkEngine_Impl()
    : m_pRaw(nullptr), m_pSsl(nullptr), m_bHasConnection(false), m_bIsServing(false)
{
    // m_trNetHandler = std::thread(&CNetworkEngine_Impl::NetworkMain_RT, this);
}

CNetworkEngine_Impl::~CNetworkEngine_Impl()
{
}

ENetworkErrorCodes CNetworkEngine_Impl::Internal_Connect()
{
    // Use reconnect vars

    if (m_bHasConnection)
    {
        return ENetworkErrorCodes::Connect_AlreadyConnectedError;
    }

    m_pRaw = BIO_new_connect((m_strHostname + ":" + std::to_string(m_uPort)).c_str());

    if (!m_pRaw)
    {
        return ENetworkErrorCodes::Connect_GeneralFailure;
    }

    auto conn = BIO_do_connect(m_pRaw);
    if (conn <= 0)
    {
        return ENetworkErrorCodes::Connect_GeneralFailure;
    }

    m_bHasConnection = true;
    return ENetworkErrorCodes::NoError;
}

ENetworkErrorCodes CNetworkEngine_Impl::SubmitTask(FTask &task, std::vector<FOperation> &vHistory)
{
    return ENetworkErrorCodes::TOTAL_ERROR_CODES;
}

ENetworkErrorCodes CNetworkEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey)
{
    m_strHostname = strHostname;
    m_uPort = uPort;
    m_vKey = vKey;

    auto ret = Internal_Connect();

    return ret;
}

ENetworkErrorCodes CNetworkEngine_Impl::RegisterNode(std::string strHostname, uint16_t uPort)
{
    m_strHostname = strHostname;
    m_uPort = uPort;

    auto ret = Internal_Connect();

    return ret;
}

ENetworkErrorCodes CNetworkEngine_Impl::Listen(uint16_t uPort)
{
    // Check port
    if (uPort == 0)
    {
        return ENetworkErrorCodes::Listen_InvalidPort;
    }

    // Try accept the port
    m_pServeRaw = BIO_new_accept(std::to_string(uPort).c_str());

    if (m_pServeRaw)
    {
        auto res = BIO_do_accept(m_pServeRaw);

        if (res <= 0)
        {
            return ENetworkErrorCodes::Listen_AcceptError;
        }

        // Handle
        // m_bIsListening = true;
    }
    else
    {
        return ENetworkErrorCodes::Listen_CreateError;
    }

    // m_pServeRaw can be read from
    // Do we want a thread for it?
    // TE already has a thread

    // Start a listening thread
    // m_trListener = std::thread(&CNetworkEngine_Impl::Listener_RT, this);

    // Get ready to handle task dispatching
    // m_trDispatch = std::thread(&CNetworkEngine_Impl::Dispatch_RT, this);

    // Fallen Through
    return ENetworkErrorCodes::NoError;
}

ENetworkErrorCodes CNetworkEngine_Impl::GetRequest(FDispatchTask &stTask)
{
    return ENetworkErrorCodes::NoError;
}

void CNetworkEngine_Impl::SubmitMessage(FMessage &&Message)
{
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    // We have no wait condition :)
    m_qMessageQueue.emplace(std::move(Message));

    // Unlock and notify
    lock.unlock();
    m_cvMessageQueue.notify_one();
}

FMessage CNetworkEngine_Impl::ReadMessage()
{
    // Wait on not being empty
    std::unique_lock<std::mutex> lock(m_mtxMessageQueue);

    m_cvMessageQueue.wait(lock, [this]() { return !m_qMessageQueue.empty(); });

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