#pragma once

#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderPass/MeshPass.h"

class Drawable
{
public:
    virtual void CollectDrawBatch(std::vector<DrawBatch>& batches) = 0;

    virtual void CollectAccelerationStructureInstance(std::vector<RHIAccelerationStructureInstanceInfo>& instances) {};
};