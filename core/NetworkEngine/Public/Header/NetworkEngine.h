#pragma once

#include "NetworkEngine/Public/Header/Defines.h"
#include "NetworkEngine/Public/Header/ErrorCode.h"
#include "TaskEngine/Public/Header/Task.h"
#include "TerrainEngine/Public/Header/Operations.h"
#include <string>
#include <vector>

NT_NS_OPEN

class NET_ENGINE_API CNetworkEngine final
{
private:
    void *m_pImplementation;

public:
    CNetworkEngine();
    ~CNetworkEngine();

    ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey);
    ENetworkErrorCodes RegisterNode(std::string strHostname, uint16_t uPort);
    ENetworkErrorCodes Listen(uint16_t uPort);

    ENetworkErrorCodes SubmitTask(FTask &task, std::vector<FOperation> &vHistory);

    // virtual ENetworkErrorCodes GetBestNetworkNode(std::vector<void*> &vNodes, NetEngComparator fnComparator);
};

NT_NS_CLOSE