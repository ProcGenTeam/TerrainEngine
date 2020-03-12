#pragma once
#include <vector>
#include <queue>
#include <cstdint>
#include <string>
#include <thread>
#include <condition_variable>

#include "NetworkEngine/Public/Header/ErrorCode.h"
#include "NetworkEngine/Private/Header/Message.h"
#include "TerrainEngine/Public/Header/Operations.h"
#include "NetworkEngine/Public/Header/NetTypes.h"
#include "NetworkEngine/Private/Header/NetworkState.h"
#include "TaskEngine/Public/Header/Task.h"

class CNetworkEngine_Impl
{
    private:
        // std::condition_variable m_cvSignal;
        // std::mutex m_mtxSignal;
        // std::thread m_trNetHandler;

        // bool m_bIsListening;
        // std::thread m_trListener;
        // std::thread m_trDispatch;

        // Message Queue
        std::condition_variable m_cvMessageQueue;
        std::mutex m_mtxMessageQueue;
        std::queue<FNetMessage> m_qMessageQueue;

        // //
        // std::vector<FNetworkState> m_vConnections;

    protected:
    public:

    private:
        //virtual void NetworkMain_RT();

        virtual void Listener_RT();
        virtual void Dispatch_RT();

        virtual void SubmitMessage(FNetMessage &&Message);
        virtual FNetMessage ReadMessage();

    protected:
    public:
        CNetworkEngine_Impl();
        virtual ~CNetworkEngine_Impl();

        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);
        virtual ENetworkErrorCodes Listen(uint16_t uPort);

        virtual ENetworkErrorCodes SubmitTask(FTask &task, std::vector<FOperation> &vHistory);

        //virtual ENetworkErrorCodes GetBestNetworkNode(std::vector<void*> &vNodes, NetEngComparator fnComparator);

};