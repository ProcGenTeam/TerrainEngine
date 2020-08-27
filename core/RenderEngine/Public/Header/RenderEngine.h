#pragma once

#include "core/Common/Public/Header/Types.h"
#include "core/RenderEngine/Public/Header/Common.h"
#include <memory>
#include <string>
#include <vector>

class CRenderEngine
{
private:
    void *m_implementation;

public:
    CRenderEngine(FCameraSettings &stSettings);
    ~CRenderEngine();

    void MarchedRender(std::string filename, std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
                       uint32_t uTerrainWidth);
};