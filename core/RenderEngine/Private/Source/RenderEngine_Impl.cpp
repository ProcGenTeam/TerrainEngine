#include "core/RenderEngine/Private/Header/RenderEngine_Impl.h"
#include "glm/geometric.hpp"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

CRenderEngine_Impl::CRenderEngine_Impl(FCameraSettings &stSettings)
{
    m_stSettings = stSettings;
}

CRenderEngine_Impl::~CRenderEngine_Impl()
{

}

glm::vec3 GetGradients(
    std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
    uint32_t uTerrainWidth,
    uint32_t uX,
    uint32_t uY
)
{
    auto lPtr = terrainView->data();

    uint32_t i = 0;
    for(int32_t i = 0; i < 4; ++i)
    {
        // Get Gradient from ind to 0
        auto offSetX = (i % 3) - 1;
        auto offSetY = (i / 3) - 1;

        auto offSetXI = ((8 - i) % 3) - 1;
        auto offSetYI = ((8 - i) / 3) - 1;
        
        // Get first index
        auto r0 = glm::vec3(uX + offSetX, uY + offSetY, lPtr[(uY + offSetY) * uTerrainWidth + (uX + offSetX)] * 255);
        auto r1 = glm::vec3(uX + offSetXI, uY + offSetYI, lPtr[(uY + offSetYI) * uTerrainWidth + (uX + offSetXI)] * 255);
        auto rR = glm::vec3(uX, uY, lPtr[uY * uTerrainWidth + uX] * 255);

        // First Gradient. r0 to rR
        auto v0 = rR - r0;
        auto v1 = rR - r1;

        // 

    }
}

glm::vec3 GetNormal(
    std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
    uint32_t uTerrainWidth,
    uint32_t uX,
    uint32_t uY
)
{
    auto lPtr = terrainView->data();

    if(uX > 0 && uX < uTerrainWidth - 1 && uY > 0 && uY < uTerrainWidth - 1)
    {


        auto rT = lPtr[(uY-1) * uTerrainWidth + uX] * 255;
        auto rB = lPtr[(uY+1) * uTerrainWidth + uX] * 255;
        auto rR = lPtr[(uY) * uTerrainWidth + (uX+1)] * 255;
        auto rL = lPtr[(uY) * uTerrainWidth + (uX-1)] * 255;

        return glm::normalize(glm::vec3(2 * (rR-rL), -4, 2 * (rB-rT)));

        //auto rC = glm::vec3(uX, uY, lPtr[uY * uTerrainWidth + uX] * 255);
    }

    return glm::vec3(0,1,0);
}

glm::vec4 CRenderEngine_Impl::MarchRay_FixedStep
(
    std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView,
    glm::vec3 &v3RayOrigin,
    glm::vec3 &v3RayDirection,
    uint32_t uTerrainWidth
)
{
    glm::vec3 v3LightLocation(m_stSettings.uWorldScaleUnits / 2, 66, m_stSettings.uWorldScaleUnits / 2);
    float fImpactLocation = m_stSettings.fFarClip;
    auto rayPos = v3RayOrigin;

    auto landPtr = terrainView->data();

    auto lCounter = 0;

    while(true)
    {
        // Check if the origin is colliding
        auto nextSample = rayPos + v3RayDirection * m_stSettings.fStepDistance;

        //uint32_t uX = uint32_t(nextSample.x);
        //uint32_t uY = uint32_t(nextSample.y);

        // Normalise to 1k grid
        auto uX = uint32_t((nextSample.x / m_stSettings.uWorldScaleUnits) * uTerrainWidth);
        auto uZ = uint32_t((nextSample.z / m_stSettings.uWorldScaleUnits) * uTerrainWidth);

        // Grid Bounds
        if(uX >= uTerrainWidth || uX < 0 || uZ >= uTerrainWidth || uZ < 0)
        {
            break;
        }

        // Landscape
        if(nextSample.y < landPtr[uZ * uTerrainWidth + uX] * 64)
        {
            //fImpactLocation = float(lCounter);//glm::distance(v3RayOrigin, rayPos);
            fImpactLocation = glm::distance(v3RayOrigin, rayPos);
            break;
        }

        rayPos = nextSample;
        ++lCounter;
        
        // Check if the sample is below the terrain
    }

    // Light Prox
    auto lightHit = sphereIntersect(v3RayOrigin, v3RayDirection, v3LightLocation, 10.f);
    if(lightHit >m_stSettings.fNearClip && lightHit < fImpactLocation)
    {
        return glm::vec4(-1);
    }

    // PatchRegion
    auto uX = uint32_t((rayPos.x / m_stSettings.uWorldScaleUnits) * uTerrainWidth);
    auto uZ = uint32_t((rayPos.z / m_stSettings.uWorldScaleUnits) * uTerrainWidth);

    //auto grad = GetGradients(terrainView, uTerrainWidth, uX, uZ);
    auto normal = GetNormal(terrainView, uTerrainWidth, uX, uZ);

    // Dot NormalVLight
    auto lAtt = glm::dot(normal, rayPos - v3LightLocation);

    return glm::vec4(lAtt);
}


// glm::vec3 ExitGradients(FGradSumParams &stParams)
// {
//     glm::vec3 refPixel = glm::vec3(
//         stParams.uX * stParams.fOneOverWidth,
//         stParams.uY * stParams.fOneOverWidth,
//         stParams.pHeight[stParams.uY * stParams.uWidth + stParams.uX]
//     );

//     glm::vec3 upDir(0,0,1);
//     glm::vec3 gradSum(0);

//     auto nBins = (m_iOverscan * 2 + 1);
//     float oneOverBoxCount = 1 / (nBins * nBins - 1);

//     // BIG NOTE
//     // This 

//     for(int32_t i = -m_iOverscan; i <= m_iOverscan; ++i)
//     {
//         int32_t modY = stParams.uY + i;
//         auto iIndex = modY * stParams.uWidth;

//         for(int32_t j = -m_iOverscan; j <= m_iOverscan; ++j)
//         {
//             auto modX = stParams.uX + j;

//             // Get this pixel
//             auto tPixel = glm::vec3(
//                 modX * stParams.fOneOverWidth,
//                 modY * stParams.fOneOverHeight,
//                 stParams.pHeight[iIndex + modX]
//             );

//             auto grad = tPixel - refPixel; 

//             grad *= (glm::dot(grad, upDir) < 0.0);

//             gradSum += grad;
//         }
//     }

//     return gradSum;
//     //return glm::normalize(gradSum);
// }

// void GenerateGradientMap(FLOAT_TYPE *pHeight, uint32_t uWidth, uint32_t uHeight)
// {
//     vv3GradientMap.resize(uWidth * uHeight);
//     auto l_pData = vv3GradientMap.data();

//     FGradSumParams stParamsDef{};
//     stParamsDef.pHeight = pHeight;
//     stParamsDef.uWidth = uWidth;
//     stParamsDef.uHeight = uHeight;
//     stParamsDef.fOneOverHeight = 1.0 / uHeight;
//     stParamsDef.fOneOverWidth = 1.0 / uWidth;

//     FGradSumParams stParams{};

// #ifdef NDEBUG
// #pragma omp parallel for private(stParams)
// #endif
//     for(uint32_t y = m_iOverscan; y < uHeight - m_iOverscan; ++y)
//     {
//         //FGradSumParams
//         stParams = stParamsDef;
//         auto my = y * uWidth;
        
//         for(uint32_t x = m_iOverscan; x < uWidth - m_iOverscan; ++x)
//         {
//             stParams.uX = x;
//             stParams.uY = y;

//             l_pData[my + x] = ExitGradients(stParams);
//         }
//     }
// }


void CRenderEngine_Impl::MarchedRender(std::string filename, std::shared_ptr<std::vector<FLOAT_TYPE>> terrainView, uint32_t uTerrainWidth)
{
    // Bad Code
    std::vector<glm::vec3> image;
    image.resize(m_stSettings.uWidth * m_stSettings.uHeight);

    // Camera
    //glm::vec3 v3CameraLocation(uTerrainWidth / 4, uTerrainWidth / 2, 64);
    glm::vec3 v3CameraLocation(0, 64, 0);
    // Lookat from origin
    glm::mat4 cameraTransform = glm::lookAtLH(v3CameraLocation, glm::vec3(m_stSettings.uWorldScaleUnits / 2, 0, m_stSettings.uWorldScaleUnits / 2), glm::vec3(0,1,0));
    cameraTransform = glm::inverse(cameraTransform);

    printf("\n%f %f %f %f\n", cameraTransform[0].r, cameraTransform[0].g, cameraTransform[0].b, cameraTransform[0].w);
    printf("%f %f %f %f\n", cameraTransform[1].r, cameraTransform[1].g, cameraTransform[1].b, cameraTransform[1].w);
    printf("%f %f %f %f\n", cameraTransform[2].r, cameraTransform[2].g, cameraTransform[2].b, cameraTransform[2].w);
    printf("%f %f %f %f\n", cameraTransform[3].r, cameraTransform[3].g, cameraTransform[3].b, cameraTransform[3].w);

    // cameraTransform = glm::mat4(1);
    // cameraTransform = glm::translate(cameraTransform, v3CameraLocation);
    // // //cameraTransform = glm::rotate(cameraTransform, 0.f, glm::vec3(1,0,0));
    // cameraTransform = glm::rotate(cameraTransform, 0.5f * 3.1415926535f, glm::vec3(1, 0, 0));
	// cameraTransform = glm::rotate(cameraTransform, 0.5f * 3.1415926535f, glm::vec3(0,1,0));

    // printf("\n%f %f %f %f\n", cameraTransform[0].r, cameraTransform[0].g, cameraTransform[0].b, cameraTransform[0].w);
    // printf("%f %f %f %f\n", cameraTransform[1].r, cameraTransform[1].g, cameraTransform[1].b, cameraTransform[1].w);
    // printf("%f %f %f %f\n", cameraTransform[2].r, cameraTransform[2].g, cameraTransform[2].b, cameraTransform[2].w);
    // printf("%f %f %f %f\n", cameraTransform[3].r, cameraTransform[3].g, cameraTransform[3].b, cameraTransform[3].w);

    //cameraTransform = glm::rotate(cameraTransform, 0.f, glm::vec3(1,0,0));
    //cameraTransform = glm::rotate(cameraTransform, -0.25f * 3.1415926535f, glm::vec3(0,1,0));
	//cameraTransform = glm::rotate(cameraTransform, 0.25f * 3.1415926535f, glm::vec3(0,0,1));



    for(uint32_t p = 0; p < image.size(); ++p)
    {
        float localSpaceGridHitX = ((float(p % m_stSettings.uWidth) / m_stSettings.uWidth) - 0.5f) * 2.f;
        float localSpaceGridHitY = ((float(p / m_stSettings.uWidth) / m_stSettings.uWidth) - 0.5f) * 2.f;

        // auto localSpaceGridHit = glm::vec4(
        //     m_stSettings.fCameraDistance, 
        //     localSpaceGridHitX * m_stSettings.fSensorSizeX,
        //     -localSpaceGridHitY * m_stSettings.fSensorSizeY,
        //     1.0f     
        //     );
        
        auto localSpaceGridHit = glm::vec4(
            localSpaceGridHitX * m_stSettings.fSensorSizeX,
            -localSpaceGridHitY * m_stSettings.fSensorSizeY,
            m_stSettings.fCameraDistance,
            1.0f     
            );

        //printf("LSG: %f %f %f %f\n", localSpaceGridHit.r, localSpaceGridHit.g, localSpaceGridHit.b, localSpaceGridHit.w);

        auto worldSpaceRayHit = cameraTransform * localSpaceGridHit;

        //printf("WSR: %f %f %f %f\n", worldSpaceRayHit.r, worldSpaceRayHit.g, worldSpaceRayHit.b, worldSpaceRayHit.w);

        auto worldSpaceRayTransform = (worldSpaceRayHit / worldSpaceRayHit.w).xyz() - v3CameraLocation;

        auto worldSpaceRayDirection = glm::normalize(
            worldSpaceRayTransform
            );

        //printf("WSD: %f %f %f\n", worldSpaceRayDirection.r, worldSpaceRayDirection.g, worldSpaceRayDirection.b);
        
        // Ray March using quanitised steps
        image[p] = MarchRay_FixedStep(
            terrainView,
            v3CameraLocation,
            worldSpaceRayDirection,
            uTerrainWidth
        ).xyz();
    }

    std::ofstream out(filename, std::ios::binary);
    out.write(
        reinterpret_cast<char *>(&m_stSettings.uWidth),
        sizeof(uint32_t)
    );
    
    out.write(
        reinterpret_cast<char *>(&m_stSettings.uHeight),
        sizeof(uint32_t)
    );

    out.write(
        reinterpret_cast<char *>(image.data()),
        image.size() * sizeof(glm::vec3)
    );
}