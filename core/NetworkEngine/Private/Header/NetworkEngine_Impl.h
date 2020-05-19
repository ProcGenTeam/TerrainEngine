#pragma once
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <vector>
#include <queue>
#include <cstdint>
#include <string>
#include <thread>
#include <condition_variable>

#include "NetworkEngine/Public/Header/ErrorCode.h"
#include "Common/Public/Header/Message.h"
#include "Common/Private/Header/Ring.h"
#include "TerrainEngine/Public/Header/Operations.h"
#include "NetworkEngine/Public/Header/NetTypes.h"
#include "NetworkEngine/Private/Header/NetworkState.h"
#include "TaskEngine/Public/Header/Task.h"

class CNetworkEngine_Impl
{
    private:
        CircularBuffer<uint64_t> m_cbData   ;
        // std::condition_variable m_cvSignal;
        // std::mutex m_mtxSignal;
        // std::thread m_trNetHandler;

        // bool m_bIsListening;
        // std::thread m_trListener;
        // std::thread m_trDispatch;

        // Message Queue
        std::condition_variable m_cvMessageQueue;
        std::mutex m_mtxMessageQueue;
        std::queue<FMessage> m_qMessageQueue;

        // //
        // std::vector<FNetworkState> m_vConnections;

        bool m_bIsServing;
        bool m_bHasConnection;

        // Connection information
        BIO *m_pRaw;
        SSL *m_pSsl;
        uint32_t m_uMemoryUsed;
        uint32_t m_uMemoryTotal;

        // Connection Serve Information
        BIO *m_pServeRaw;

        // Reconnection information
        std::string m_strHostname;
        uint16_t m_uPort;
        std::vector<char> m_vKey;
        //__uint128_t m_uKey;

    protected:
    public:

    private:
        //virtual void NetworkMain_RT();

        virtual void Listener_RT();
        virtual void Dispatch_RT();

        virtual void SubmitMessage(FMessage &&Message);
        virtual FMessage ReadMessage();

        virtual ENetworkErrorCodes Internal_Connect();

    protected:
    public:
        CNetworkEngine_Impl();
        virtual ~CNetworkEngine_Impl();

        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);
        virtual ENetworkErrorCodes Listen(uint16_t uPort);

        // Caller allocates
        virtual ENetworkErrorCodes GetRequest(FDispatchTask &stTask);

        virtual ENetworkErrorCodes SubmitTask(FTask &task, std::vector<FOperation> &vHistory);

        //virtual ENetworkErrorCodes GetBestNetworkNode(std::vector<void*> &vNodes, NetEngComparator fnComparator);
};