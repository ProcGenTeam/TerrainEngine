#include "RenderEngine/Public/Header/RenderEngine.h"
#include "RenderEngine/Private/Header/RenderEngine_Impl.h"

#define Super (reinterpret_cast<CRenderEngine_Impl *>(this->m_implementation))

CRenderEngine::CRenderEngine(FCameraSettings &stSettings) : m_implementation(new CRenderEngine_Impl(stSettings))
{
}

CRenderEngine::~CRenderEngine()
{
    delete Super;
}

void CRenderEngine::MarchedRender(std::string filename, std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
                                  uint32_t uTerrainWidth)
{
    Super->MarchedRender(filename, terrainView, uTerrainWidth);
}
