#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "core/Common/Public/Header/Types.h"
#include "core/RenderEngine/Public/Header/Common.h"
#include "core/RenderEngine/Private/Header/RenderHelpers.h"

class CRenderEngine_Impl
{
    private:
    
        FCameraSettings m_stSettings;
        
        glm::vec4 MarchRay_FixedStep(
            std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
            glm::vec3 &v3RayOrigin,
            glm::vec3 &v3RayDirection,
            uint32_t uTerrainwidth
            );

    public:
        CRenderEngine_Impl(FCameraSettings &stSettings);
        ~CRenderEngine_Impl();

        void MarchedRender(std::string filename, std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView, uint32_t uTerrainWidth);
};
