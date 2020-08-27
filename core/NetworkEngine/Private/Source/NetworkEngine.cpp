#include "NetworkEngine/Public/Header/NetworkEngine.h"
#include "NetworkEngine/Private/Header/NetworkEngine_Impl.h"
#include "NetworkEngine/Public/Header/ErrorCode.h"

#define Super (reinterpret_cast<CNetworkEngine_Impl *>(this->m_pImplementation))

TerrainEngine::CNetworkEngine::CNetworkEngine() : m_pImplementation(new CNetworkEngine_Impl())
{
}

TerrainEngine::CNetworkEngine::~CNetworkEngine()
{
    delete Super;
}

ENetworkErrorCodes TerrainEngine::CNetworkEngine::Listen(uint16_t uPort)
{
    return Super->Listen(uPort);
}

ENetworkErrorCodes TerrainEngine::CNetworkEngine::RegisterNode(std::string strHostname, uint16_t uPort,
                                                               std::vector<char> vKey)
{
    return Super->RegisterNode(strHostname, uPort, vKey);
}

ENetworkErrorCodes TerrainEngine::CNetworkEngine::RegisterNode(std::string strHostname, uint16_t uPort)
{
    return Super->RegisterNode(strHostname, uPort);
}