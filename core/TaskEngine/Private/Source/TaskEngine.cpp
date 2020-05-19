#include "TaskEngine/Public/Header/TaskEngine.h"
#include "TaskEngine/Private/Header/TaskEngine_Impl.h"

#define Super (reinterpret_cast<CTaskEngine_Impl*>(this->m_implementation))

TerrainEngine::CTaskEngine::CTaskEngine(uint64_t uWidth, uint64_t uHeight, uint32_t uTileSize)
 : m_implementation(new CTaskEngine_Impl(uWidth, uHeight, uTileSize))
{

}

TerrainEngine::CTaskEngine::~CTaskEngine()
{
    delete Super;
}

ETaskErrorCodes TerrainEngine::CTaskEngine::Render(std::vector<FOperation> &vHistory)
{
    return Super->Render(vHistory);
}

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey)
{
    return Super->RegisterNode(strHostname, uPort, vKey);
}

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t uPort)
{
    return Super->RegisterNode(strHostname, uPort);
}

ETaskErrorCodes TerrainEngine::CTaskEngine::Listen(uint16_t uPort)
{
    return Super->Listen(uPort);
}

