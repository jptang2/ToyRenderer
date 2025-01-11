
#include "Core/Math/Math.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/SkyboxComponent.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderPass/RenderPass.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Runtime/Function/Framework/Component/VolumeLightComponent.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

void LoadScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->LoadScene("Asset/BuildIn/Scene/default.asset");
    EngineContext::World()->SetActiveScene(scene->GetName());
}

void InitStressTestScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->CreateNewScene("stressTestScene");
    EngineContext::Render()->GetGlobalSetting()->minFrameTime = 30.0f;  // 设置一个最小帧时间
    // 主要的开销全在点光源和平行光源的阴影上了

    // Directional light
    if(true)
    {
        std::shared_ptr<Entity> directionalLight                                = scene->CreateEntity("Directional Light"); 
        std::shared_ptr<TransformComponent> transformComponent                  = directionalLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<DirectionalLightComponent> directionalLightComponent    = directionalLight->AddComponent<DirectionalLightComponent>();

        transformComponent->SetRotation({0.0f, 30.0f, -60.0f});
        directionalLightComponent->SetUpdateFrequency(0, 1);
        directionalLightComponent->SetUpdateFrequency(1, 2);
        directionalLightComponent->SetUpdateFrequency(2, 5);
        directionalLightComponent->SetUpdateFrequency(3, 10);
    }

    // Point light
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Light");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-2.0f, 3.0f, 10.0f});
        pointLightComponent->SetScale(25.0f);
        pointLightComponent->SetColor(Vec3(0.5f, 0.88f, 1.0f));
        pointLightComponent->SetIntencity(50.0f);
        pointLightComponent->SetCastShadow(false);
    }

    // Camera
    if(true)
    {
        std::shared_ptr<Entity> camera                                  = scene->CreateEntity("Camera");
        std::shared_ptr<TransformComponent> transformComponent          = camera->TryGetComponent<TransformComponent>();
        std::shared_ptr<CameraComponent> cameraComponent                = camera->AddComponent<CameraComponent>();
        std::shared_ptr<SkyboxComponent> skyboxComponent                = camera->AddComponent<SkyboxComponent>();

        cameraComponent->SetFov(75.0f);

        transformComponent->SetPosition({-2.0f, 3.0f, 11.0f});
        transformComponent->SetRotation({0.0f, 37.0f, 5.0f});

        std::vector<std::string> skyboxPaths;
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/px.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nx.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/py.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/ny.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/pz.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nz.png");
        TextureRef skyboxTexture = std::make_shared<Texture>(skyboxPaths, TEXTURE_TYPE_CUBE);
        EngineContext::Asset()->SaveAsset(skyboxTexture, "Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");
        // TextureRef skyboxTexture = EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");

        skyboxComponent->SetSkyboxTexture(skyboxTexture);
    }

    // Bunny
    MaterialRef material = std::make_shared<Material>();
    for(int i = 0; i < 1000; i++)
    {
        Vec3 pos = Vec3((i / 100) % 10, (i / 10) % 10, i % 10);

        std::shared_ptr<Entity> model2                                  = scene->CreateEntity("Bunny" + std::to_string(i));
        std::shared_ptr<MeshRendererComponent> meshRendererComponent2   = model2->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = model2->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition(pos);
        transformComponent->SetScale({5.0f, 5.0f, 5.0f});

        std::shared_ptr<Model> model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Basic/stanford_bunny.asset");
        meshRendererComponent2->SetModel(model);
        meshRendererComponent2->SetMaterial(material); 
    }

    EngineContext::World()->SetActiveScene("stressTestScene");
}

void InitScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->CreateNewScene("defaultScene");

    // Camera
    if(true)
    {
        std::shared_ptr<Entity> camera                                  = scene->CreateEntity("Camera");
        std::shared_ptr<TransformComponent> transformComponent          = camera->TryGetComponent<TransformComponent>();
        std::shared_ptr<CameraComponent> cameraComponent                = camera->AddComponent<CameraComponent>();

        // transformComponent->SetPosition({-30.767f, 5.027f, 12.705f});
        // transformComponent->SetRotation({0.0f, 17.0f, -24.0f});

        transformComponent->SetPosition({-13.898f, 3.495f, -10.011f});
        transformComponent->SetRotation({0.0f, -58.5f, -8.5f});
    }

    // Directional light
    if(true)
    {
        std::shared_ptr<Entity> directionalLight                                = scene->CreateEntity("Directional Light"); 
        std::shared_ptr<TransformComponent> transformComponent                  = directionalLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<DirectionalLightComponent> directionalLightComponent    = directionalLight->AddComponent<DirectionalLightComponent>();
        std::shared_ptr<SkyboxComponent> skyboxComponent                        = directionalLight->AddComponent<SkyboxComponent>();

        transformComponent->SetPosition({-20.0f, 3.0f, 0.0f});
        transformComponent->SetRotation({0.0f, 30.0f, -60.0f});

        directionalLightComponent->SetIntencity(3.7f);
        // directionalLightComponent->SetFogScattering(0.0f);

        std::vector<std::string> skyboxPaths;
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/px.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nx.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/py.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/ny.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/pz.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nz.png");
        TextureRef skyboxTexture = std::make_shared<Texture>(skyboxPaths, TEXTURE_TYPE_CUBE);
        EngineContext::Asset()->SaveAsset(skyboxTexture, "Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");
        // TextureRef skyboxTexture = EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");

        skyboxComponent->SetSkyboxTexture(skyboxTexture);
        skyboxComponent->SetIntencity(1.0f);
    }

    // Volume light 0
    if(true)
    {
        std::shared_ptr<Entity> volumeLight                             = scene->CreateEntity("Volume light 0"); 
        std::shared_ptr<TransformComponent> transformComponent          = volumeLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<VolumeLightComponent> volumeLightComponent      = volumeLight->AddComponent<VolumeLightComponent>();

        transformComponent->SetPosition({8.6f, -4.0f, -4.9f});

        volumeLightComponent->SetEnergyPreservation(0.95f);
        volumeLightComponent->SetUpdateFrequence(0);
        volumeLightComponent->SetProbeCounts({21, 10, 18});
        volumeLightComponent->SetGridStep({6.0f, 6.0f, 6.0f});
        volumeLightComponent->SetVisulaize(false);
    }

    // Volume light 1
    if(true)
    {
        std::shared_ptr<Entity> volumeLight                             = scene->CreateEntity("Volume light 1"); 
        std::shared_ptr<TransformComponent> transformComponent          = volumeLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<VolumeLightComponent> volumeLightComponent      = volumeLight->AddComponent<VolumeLightComponent>();

        transformComponent->SetPosition({-70.0f, 13.0f, 2.85f});

        volumeLightComponent->SetEnergyPreservation(0.95f);
        volumeLightComponent->SetUpdateFrequence(0);
        volumeLightComponent->SetVisulaize(false);
    }


    // Point Shadow light
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Shadow Light 1");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-20.0f, 3.0f, 8.5f});
        pointLightComponent->SetScale(15.0f);
        pointLightComponent->SetColor(Vec3(0.0f, 0.285f, 1.0f));
        pointLightComponent->SetIntencity(100.0f);
        // pointLightComponent->SetFogScattering(0.5f);
    }
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Shadow Light 2");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-3.0f, 5.5f, 10.0f});
        pointLightComponent->SetScale(40.0f);
        pointLightComponent->SetColor(Vec3(1.0f, 0.66f, 0.0f));
        pointLightComponent->SetIntencity(100.0f);
        pointLightComponent->SetFogScattering(0.2f);
    }


    // Point light
    for(int i = 0; i < 1000; i++)
    {
        Vec3 pos = Vec3((i / 100) % 10, (i / 10) % 10, i % 10) * 5.0f;
        pos += Vec3(-25.0f, 5.0f, -10.0f);
        Vec3 color = Vec3((i / 100) % 10, (i / 10) % 10, i % 10) / 10.0f;

        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Light " + std::to_string(i + 1));
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition(pos);
        pointLightComponent->SetScale(25.0f);
        pointLightComponent->SetColor(color);
        pointLightComponent->SetIntencity(50.0f);
        pointLightComponent->SetCastShadow(false);
    }

    // Sphere
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Sphere");
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-20.4f, 3.4f, 11.2f});
        transformComponent->SetRotation({0.0f, 0.0f, 0.0f});
        transformComponent->SetScale({1.0f, 1.0f, 1.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = true,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = true,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/sphere_div.obj", processSetting);

        MaterialRef material = std::make_shared<Material>();
        material->SetMetallic(1.0f);
        material->SetRoughness(0.05f);

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials({material});
    }

    // Plane 1
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Plane 1");
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-2.0f, -0.4f, 1.9f});
        transformComponent->SetRotation({0.0f, 0.0f, 0.0f});
        transformComponent->SetScale({30.0f, 5.0f, 20.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = true,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/plane.obj", processSetting);

        MaterialRef material = std::make_shared<Material>();
        // material->SetMetallic(0.2f);
        // material->SetRoughness(0.05f);
        material->SetMetallic(0.7f);
        material->SetRoughness(0.04f);

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials({material});
    }

    // Plane 2
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Plane 2");
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-27.0f, 2.5f, -6.0f});
        transformComponent->SetRotation({90.0f, 0.0f, 0.0f});
        transformComponent->SetScale({3.0f, 3.0f, 3.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = true,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = true,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/plane.obj", processSetting);

        TextureRef texture = std::make_shared<Texture>("Asset/BuildIn/Texture/basic/checker.png");

        MaterialRef material = std::make_shared<Material>();
        material->SetDiffuse(texture);
        material->SetMetallic(0.0f);
        material->SetRoughness(1.0f);

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials({material});
    }

    // emmisive box
    if(true)
    {
        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/cube.obj", processSetting);

        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("emmisive box");
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-3.0, 3.5, 13.0});
        transformComponent->SetRotation({0.0f, 0.0f, 0.0f});

        MaterialRef material = std::make_shared<Material>();
        material->SetMetallic(0.0f);
        material->SetRoughness(1.0f);
        material->SetEmission({0.8f, 1.0f, 0.8f, 20.0f});

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials({material});
    }


    // Cornell Box
    if(true)
    {
        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/cube.obj", processSetting);

        // 下 左 右 前 后 上
        Vec3 positions[6] = {   Vec3(-70.0, 0.0, 0.0),
                                Vec3(-70.0, 9.5, 9.5),
                                Vec3(-70.0, 9.5, -9.5),
                                Vec3(-79.5, 9.5, 0.0),
                                Vec3(-60.5, 9.5, 0.0),
                                Vec3(-70.0, 19.0, 5.0)};

        Vec3 scales[6] = {  Vec3(10.0, 0.5, 10.0),
                            Vec3(10.0, 10.0, 0.5),
                            Vec3(10.0, 10.0, 0.5),
                            Vec3(0.5, 10.0, 10.0),
                            Vec3(0.5, 10.0, 10.0),
                            Vec3(10.0, 0.5, 10.0)};

        Vec4 colors[6] = {  Vec4(1.0, 1.0, 1.0, 1.0),
                            Vec4(0.12, 0.45, 0.15, 1.0), // green
                            Vec4(0.65, 0.05, 0.05, 1.0), // red
                            Vec4(1.0, 1.0, 1.0, 1.0),
                            Vec4(1.0, 1.0, 1.0, 1.0),
                            Vec4(1.0, 1.0, 1.0, 1.0)};                   

        for(int i = 0; i < 6; i++)
        {
            std::shared_ptr<Entity> entity                                  = scene->CreateEntity("box " + std::to_string(i));
            std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
            std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();

            transformComponent->SetPosition(positions[i]);
            transformComponent->SetRotation({0.0f, 0.0f, 0.0f});
            transformComponent->SetScale(scales[i]);

            MaterialRef material = std::make_shared<Material>();
            material->SetDiffuse(colors[i]);
            // material->SetMetallic(1.0f);
            // material->SetRoughness(0.05f);
            material->SetMetallic(0.0f);
            material->SetRoughness(1.0f);

            meshRendererComponent->SetModel(model);
            meshRendererComponent->SetMaterials({material});
        }
    }


    // Klee
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Klee");
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent   = entity->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-20.1f, -0.2f, 7.9f});
        transformComponent->SetRotation({0.0f, -90.0f, 0.0f});
        transformComponent->SetScale({2.0f, 2.0f, 2.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Klee/klee.fbx", processSetting);

        std::vector<TextureRef> textures;
        textures.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/face.jpg")); 
        textures.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/hair.jpg")); 
        textures.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/dressing.jpg"));  

        std::vector<MaterialRef> materials1;
        materials1.push_back(std::make_shared<Material>());
        materials1.push_back(std::make_shared<Material>());
        materials1.push_back(std::make_shared<Material>());
        materials1[0]->SetDiffuse(textures[0]);
        materials1[1]->SetDiffuse(textures[1]);
        materials1[2]->SetDiffuse(textures[2]);

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials(materials1);
    }

    // Bunny
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Bunny");
        std::shared_ptr<MeshRendererComponent> meshRendererComponent    = entity->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-20.2f, -0.65f, 9.6f});
        transformComponent->SetScale({10.0f, 10.0f, 10.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = true,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = true,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/stanford_bunny.obj", processSetting);

        MaterialRef material = std::make_shared<Material>();

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterial(material); 
    }

    // Scene
    if(true)
    {
        std::shared_ptr<Entity> entity                                  = scene->CreateEntity("Scene");
        std::shared_ptr<MeshRendererComponent> meshRendererComponent   = entity->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = entity->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({0.0f, 0.0f, 0.0f});
        transformComponent->SetScale({0.5f, 0.5f, 0.5f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = true,
            .loadMaterials = true,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = true,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Scene/scene_fix_no_tree.fbx", processSetting);
        
        std::vector<MaterialRef> materials;
        for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
        {
            //materials.push_back(std::make_shared<Material>());
            materials.push_back(model->GetMaterial(i));

            // auto material = materials.back();
            // material->SetMetallic(0.9f);
            // material->SetRoughness(0.05f);
        }

        meshRendererComponent->SetModel(model);
        meshRendererComponent->SetMaterials(materials);
    }

    // EngineContext::Render()->SetPassEnabled(FORWARD_PASS, false);
    // EngineContext::Render()->SetPassEnabled(EDITOR_UI_PASS, false);
    // EngineContext::Render()->SetPassEnabled(IBL_PASS, false);
    // EngineContext::Render()->SetPassEnabled(SVGF_PASS, false);
    // EngineContext::Render()->SetPassEnabled(G_BUFFER_PASS, false);
    // EngineContext::Render()->SetPassEnabled(FORWARD_PASS, false);
    // EngineContext::Render()->SetPassEnabled(TRANSPARENT_PASS, false);
    // EngineContext::Render()->SetPassEnabled(DDGI_VISUALIZE_PASS, false);
    // EngineContext::Render()->SetPassEnabled(PATH_TRACING_PASS, false);
    // EngineContext::Render()->SetPassEnabled(BLOOM_PASS, false);
    // EngineContext::Render()->SetPassEnabled(FXAA_PASS, false);
    // EngineContext::Render()->SetPassEnabled(TAA_PASS, false);
    // EngineContext::Render()->SetPassEnabled(SSSR_PASS, false);
    // EngineContext::Render()->SetPassEnabled(VOLUMETIRC_FOG_PASS, false);
    // EngineContext::Render()->SetPassEnabled(RESTIR_PASS, false);
    EngineContext::World()->SetActiveScene("defaultScene");
}

int main() 
{
    EngineContext::Init();

    //InitStressTestScene();
    InitScene();
    //LoadScene();

    EngineContext::MainLoop();
    EngineContext::Destroy();
    return 0;
}



