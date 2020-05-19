#pragma once

struct FCameraSettings
{
    float fSensorSizeX;
    float fSensorSizeY;
    float fCameraDistance;

    float fStepDistance;
    
    float fFarClip;
    float fNearClip;

    uint32_t uWorldScaleUnits;

    uint32_t uWidth;
    uint32_t uHeight;

};