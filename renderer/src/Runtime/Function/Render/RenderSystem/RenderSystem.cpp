#include "RenderSystem.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderPass/GPUCullingPass.h"
#include "Function/Render/RenderPass/ClusterLightingPass.h"
#include "Function/Render/RenderPass/IBLPass.h"
#include "Function/Render/RenderPass/DepthPass.h"
#include "Function/Render/RenderPass/DepthPyramidPass.h"
#include "Function/Render/RenderPass/DirectionalShadowPass.h"
#include "Function/Render/RenderPass/PointShadowPass.h"
#include "Function/Render/RenderPass/ClipmapPass.h"
#include "Function/Render/RenderPass/DDGIPass.h"
#include "Function/Render/RenderPass/GBufferPass.h"
#include "Function/Render/RenderPass/SurfaceCachePass.h"
#include "Function/Render/RenderPass/ReprojectionPass.h"
#include "Function/Render/RenderPass/DeferredLightingPass.h"
#include "Function/Render/RenderPass/SSSRPass.h"
#include "Function/Render/RenderPass/VolumetricFogPass.h"
#include "Function/Render/RenderPass/ReSTIRDIPass.h"
#include "Function/Render/RenderPass/ReSTIRGIPass.h"
#include "Function/Render/RenderPass/SVGFPass.h"
#include "Function/Render/RenderPass/ForwardPass.h"
#include "Function/Render/RenderPass/ClipmapVisualizePass.h"
#include "Function/Render/RenderPass/DDGIVisualizePass.h"
#include "Function/Render/RenderPass/BloomPass.h"
#include "Function/Render/RenderPass/FXAAPass.h"
#include "Function/Render/RenderPass/TAAPass.h"
#include "Function/Render/RenderPass/ExposurePass.h"
#include "Function/Render/RenderPass/PostProcessingPass.h"
#include "Function/Render/RenderPass/RayTracingBasePass.h"
#include "Function/Render/RenderPass/PathTracingPass.h"
#include "Function/Render/RenderPass/GizmoPass.h"
#include "Function/Render/RenderPass/EditorUIPass.h"
#include "Function/Render/RenderPass/PresentPass.h"
#include "Function/Render/RenderPass/RenderPass.h"
#include "RenderSurfaceCacheManager.h"
#include <memory>

void RenderSystem::InitGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_EXTENT.width, WINDOW_EXTENT.height, "Toy Renderer", nullptr, nullptr); 
    // glfwSetWindowUserPointer(m_window, this);
    // glfwSetWindowSizeCallback(m_window, nullptr); //TODO
    // glfwSetCursorPosCallback(m_window, MousePosCallback);
    // glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    // glfwSetScrollCallback(m_window, ScrollCallback);
    // glfwSetKeyCallback(m_window, KeyCallback);
}

void RenderSystem::DestroyGLFW()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void RenderSystem::Init() 
{ 
    EngineContext::RHI()->InitImGui(window);

    lightManager = std::make_shared<RenderLightManager>();
    meshManager = std::make_shared<RenderMeshManager>();
    surfaceCacheManager = std::make_shared<RenderSurfaceCacheManager>();
    lightManager->Init();
    meshManager->Init();
    surfaceCacheManager->Init();

    InitBaseResource(); 
    InitPasses();  
}

void RenderSystem::InitBaseResource()
{
    backend       = EngineContext::RHI();
    surface       = backend->CreateSurface(window);
    queue         = backend->GetQueue({ QUEUE_TYPE_GRAPHICS, 0 });
    swapchain     = backend->CreateSwapChain({ surface, queue, FRAMES_IN_FLIGHT, surface->GetExetent(), COLOR_FORMAT });
    pool          = backend->CreateCommandPool({ queue });  
    for(uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) 
    {
        perFrameCommonResources[i].command = pool->CreateCommandList(true);
        perFrameCommonResources[i].startSemaphore = backend->CreateSemaphore();
        perFrameCommonResources[i].finishSemaphore = backend->CreateSemaphore();
        perFrameCommonResources[i].fence = backend->CreateFence(true);
    }
}

void RenderSystem::InitPasses()
{
    meshPasses[MESH_DEPTH_PASS]                 = std::make_shared<DepthPass>();
    meshPasses[MESH_DIRECTIONAL_SHADOW_PASS]    = std::make_shared<DirectionalShadowPass>();
    meshPasses[MESH_POINT_SHADOW_PASS]          = std::make_shared<PointShadowPass>();
    meshPasses[MESH_G_BUFFER_PASS]              = std::make_shared<GBufferPass>();
    meshPasses[MESH_FORWARD_PASS]               = std::make_shared<ForwardPass>();

    passes[GPU_CULLING_PASS]                    = std::make_shared<GPUCullingPass>();
    passes[CLUSTER_LIGHTING_PASS]               = std::make_shared<ClusterLightingPass>();
    passes[IBL_PASS]                            = std::make_shared<IBLPass>();
    passes[DEPTH_PASS]                          = meshPasses[MESH_DEPTH_PASS];
    passes[DEPTH_PYRAMID_PASS]                  = std::make_shared<DepthPyramidPass>();
    passes[POINT_SHADOW_PASS]                   = meshPasses[MESH_POINT_SHADOW_PASS];
    passes[DIRECTIONAL_SHADOW_PASS]             = meshPasses[MESH_DIRECTIONAL_SHADOW_PASS];
    passes[CLIPMAP_PASS]                        = nullptr;
    passes[DDGI_PASS]                           = nullptr;
    passes[G_BUFFER_PASS]                       = meshPasses[MESH_G_BUFFER_PASS];
    passes[SURFACE_CACHE_PASS]                  = nullptr;
    passes[REPROJECTION_PASS]                   = std::make_shared<ReprojectionPass>();
    passes[DEFERRED_LIGHTING_PASS]              = std::make_shared<DeferredLightingPass>();
    passes[RESTIR_DI_PASS]                      = nullptr;
    passes[RESTIR_GI_PASS]                      = nullptr;
    passes[SVGF_PASS]                           = nullptr;
    passes[SSSR_PASS]                           = std::make_shared<SSSRPass>();
    passes[VOLUMETIRC_FOG_PASS]                 = std::make_shared<VolumetricFogPass>();
    passes[FORWARD_PASS]                        = meshPasses[MESH_FORWARD_PASS];
    passes[CLIPMAP_VISUALIZE_PASS]              = nullptr;
    passes[DDGI_VISUALIZE_PASS]                 = nullptr;
    passes[TRANSPARENT_PASS]                    = nullptr;
    passes[PATH_TRACING_PASS]                   = nullptr;
    passes[BLOOM_PASS]                          = std::make_shared<BloomPass>();
    passes[FXAA_PASS]                           = std::make_shared<FXAAPass>();
    passes[TAA_PASS]                            = std::make_shared<TAAPass>();
    passes[EXPOSURE_PASS]                       = std::make_shared<ExposurePass>();
    passes[POST_PROCESSING_PASS]                = std::make_shared<PostProcessingPass>();
    passes[RAY_TRACING_BASE_PASS]               = nullptr;
    passes[GIZMO_PASS]                          = std::make_shared<GizmoPass>();
    passes[EDITOR_UI_PASS]                      = std::make_shared<EditorUIPass>();
    passes[PRESENT_PASS]                        = std::make_shared<PresentPass>();

    passes[BLOOM_PASS]->SetEnable(false);       //TODO 
    passes[VOLUMETIRC_FOG_PASS]->SetEnable(false);

#if ENABLE_RAY_TRACING
    passes[CLIPMAP_PASS]                    = std::make_shared<ClipmapPass>(); 
    passes[DDGI_PASS]                       = std::make_shared<DDGIPass>();  
    passes[SURFACE_CACHE_PASS]              = std::make_shared<SurfaceCachePass>();
    passes[RESTIR_DI_PASS]                  = std::make_shared<ReSTIRDIPass>();
    passes[RESTIR_GI_PASS]                  = std::make_shared<ReSTIRGIPass>();
    passes[SVGF_PASS]                       = std::make_shared<SVGFPass>();
    passes[CLIPMAP_VISUALIZE_PASS]          = std::make_shared<ClipmapVisualizePass>();
    passes[DDGI_VISUALIZE_PASS]             = std::make_shared<DDGIVisualizePass>();
    passes[PATH_TRACING_PASS]               = std::make_shared<PathTracingPass>();
    passes[RAY_TRACING_BASE_PASS]           = std::make_shared<RayTracingBasePass>();

    passes[CLIPMAP_PASS]->SetEnable(false);
    passes[CLIPMAP_VISUALIZE_PASS]->SetEnable(false);
    passes[RESTIR_DI_PASS]->SetEnable(false);
    passes[PATH_TRACING_PASS]->SetEnable(false);  
    passes[RAY_TRACING_BASE_PASS]->SetEnable(false);      
#endif

    for(auto& pass : passes) { if(pass) pass->Init(); }
}

bool RenderSystem::Tick()
{
    ENGINE_TIME_SCOPE(RenderSystem::Tick);
    if(!glfwWindowShouldClose(window))
    {
        if(EngineContext::World()->GetActiveScene() == nullptr) return false;

        meshManager->Tick();            // 先准备各个meshpass的绘制信息
        lightManager->Tick();           // 准备光源信息   
        surfaceCacheManager->Tick();    // 更新surfaceCache
        UpdateGlobalSetting();                   

        auto& resource = perFrameCommonResources[EngineContext::CurrentFrameIndex()];

        resource.fence->Wait();                         // 等待帧栅栏
        RHITextureRef swapchainTexture = swapchain->GetNewFrame(nullptr, resource.startSemaphore); 
        RHICommandListRef command = resource.command;   // 构建RDG，绘制提交
        command->BeginCommand();
        {
            ENGINE_TIME_SCOPE(RenderSystem::RDGBuild);
            RDGBuilder rdgBuilder = RDGBuilder(command);
            
            for(auto& pass : passes) { if(pass) pass->Build(rdgBuilder); }
            rdgBuilder.Execute();
            rdgDependencyGraph = rdgBuilder.GetGraph(); //
        }
        {
            ENGINE_TIME_SCOPE(RenderSystem::RecordCommands);
            command->EndCommand();
            command->Execute(resource.fence, resource.startSemaphore, resource.finishSemaphore);
        }      
        swapchain->Present(resource.finishSemaphore); 
        return false; 
    }
    return true;
}

void RenderSystem::UpdateGlobalSetting()
{
    globalSetting.totalTicks = EngineContext::GetCurretTick();
    globalSetting.totalTickTime += EngineContext::GetDeltaTime();
    globalSetting.skyboxMaterialID = EngineContext::World()->GetActiveScene()->GetSkyBox() ?
                                     EngineContext::World()->GetActiveScene()->GetSkyBox()->GetMaterialID() : 0;
    //globalSetting.clusterInspectMode;   // 在Editor里设置
    EngineContext::RenderResource()->SetRenderGlobalSetting(globalSetting);
}

