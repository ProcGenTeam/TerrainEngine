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

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t uPort, std::vector<char> vKey)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t uPort)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

ETaskErrorCodes TerrainEngine::CTaskEngine::Listen(uint16_t uPort)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

