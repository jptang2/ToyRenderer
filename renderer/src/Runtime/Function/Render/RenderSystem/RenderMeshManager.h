#pragma once

#include "Function/Render/RHI/RHIStructs.h"
class RenderMeshManager
{
public:
    void Init();
    void Tick();

private:
    void PrepareMeshPass();
    void PrepareRayTracePass();

    RHITopLevelAccelerationStructureRef tlas;
};