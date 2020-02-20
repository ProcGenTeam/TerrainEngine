#include "TaskEngine/Public/Header/TaskEngine.h"
#include "TaskEngine/Private/Header/TaskEngine_Impl.h"

#define Super (reinterpret_cast<CTaskEngine_Impl*>(this->m_implementation))

TerrainEngine::CTaskEngine::CTaskEngine()
 : m_implementation(new CTaskEngine_Impl())
{

}

TerrainEngine::CTaskEngine::~CTaskEngine()
{
    delete Super;
}

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t sPort, std::vector<char> vKey)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

ETaskErrorCodes TerrainEngine::CTaskEngine::RegisterNode(std::string strHostname, uint16_t sPort)
{
    return ETaskErrorCodes::TOTAL_ERROR_CODES;
}

