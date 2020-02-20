#pragma once

#include "TaskEngine/Public/Header/Defines.h"
#include "TaskEngine/Public/Header/ErrorCode.h"
#include <cstdint>
#include <vector>
#include <string>

TS_NS_OPEN

class TASK_ENGINE_API CTaskEngine
{
    private:
        void *m_implementation;
    public:
    // Methods
        CTaskEngine();
        ~CTaskEngine();

        // Get View to preview state
        // 
        //std::shared_ptr<std::vector<uint32_t>> GetView();

        virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t sPort, std::vector<char> vKey);
        virtual ETaskErrorCodes RegisterNode(std::string strHostname, uint16_t sPort);
};

TS_NS_CLOSE