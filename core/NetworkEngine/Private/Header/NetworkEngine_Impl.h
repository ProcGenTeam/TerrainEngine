#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <thread>
#include <condition_variable>

#include "NetworkEngine/Public/Header/ErrorCode.h"

class CNetworkEngine_Impl
{
    private:
        std::condition_variable m_cvSignal;
        std::mutex m_mtxSignal;
        std::thread m_trNetHandler;

        bool bIsListening;
        std::thread m_trListener;
    protected:
    public:

    private:
        virtual void NetworkMain_RT();

        virtual void Listener_RT();

    protected:
    public:
        CNetworkEngine_Impl();
        virtual ~CNetworkEngine_Impl();

        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
        virtual ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);
        virtual ENetworkErrorCodes Listen(uint16_t uPort);
};