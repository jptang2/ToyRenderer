#pragma once

#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Framework/Component/VolumeLightComponent.h"
#include <memory>
#include <vector>
class RenderLightManager
{
public:
    void Init();
    void Tick();

    inline std::shared_ptr<DirectionalLightComponent> GetDirectionalLight()                 { return directionalLight; }
    inline const std::vector<std::shared_ptr<PointLightComponent>>& GetPointShadowLights()  { return pointShadowLights; }
    inline const std::vector<std::shared_ptr<VolumeLightComponent>>& GetVolumeLights()      { return volumeLights; }

private:
    void PrepareLights();

    std::vector<std::shared_ptr<PointLightComponent>> pointShadowLights;
    std::shared_ptr<DirectionalLightComponent> directionalLight;
    std::vector<std::shared_ptr<VolumeLightComponent>> volumeLights;
};