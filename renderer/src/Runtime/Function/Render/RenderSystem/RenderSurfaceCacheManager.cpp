#include "RenderSurfaceCacheManager.h"
#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/SurfaceCache/SurfaceCache.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <minwindef.h>
#include <vector>

void RenderSurfaceCacheManager::Init()
{
    cache = std::vector<MeshCardCache>(MAX_PER_FRAME_OBJECT_SIZE * 6 + 1); 
    sortedID = std::vector<uint32_t>(MAX_PER_FRAME_OBJECT_SIZE * 6 + 1);
    std::iota(sortedID.begin(), sortedID.end(), 0);
}

void RenderSurfaceCacheManager::Tick()
{
    UpdateSurfaceCache();
}

void RenderSurfaceCacheManager::ReleaseCache(uint32_t meshCardID)
{
    if(cache[meshCardID].valid)
    {
        objectIDtoCardID.erase(cache[meshCardID].objectID);
        for(int face = 0; face < 6; face++)
        {
            auto& entry = cache[meshCardID + face];
            if(entry.atlasRange) atlas.Release(entry.atlasRange); // 释放cache

            cache[meshCardID + face].valid = false;
        }
    }
    if(maxUsedMeshCardID == meshCardID) 
    {
        maxUsedMeshCardID -= 6;
        std::iota(sortedID.begin(), sortedID.end(), 0);     // 如果最大的已使用meshCardID范围变化，需要重新初始化一次排序下标
    }                                                                        // 否则就可以保持排序状态，在下一帧排序时已经部分有序？不确定是否有效
}

void RenderSurfaceCacheManager::InitCache(const SurfaceCacheTask& task)
{
    objectIDtoCardID[task.objectID] = task.meshCardID;
    maxUsedMeshCardID = std::max(maxUsedMeshCardID, task.meshCardID);

    Vec3 extent = task.box.maxBound - task.box.minBound;
    for(int i = 0; i < 3; i++) extent[i] = std::max(extent[i], 0.05f);  // 取个下限避免面片的问题

    Vec3 center = (task.box.maxBound + task.box.minBound) / 2;
    extent = extent.array() * task.scale.array();   // 按实际尺寸来计算
    center = center.array() * task.scale.array(); 

    static const Vec3 direction[6] = {
        Vec3::UnitX(),
        -Vec3::UnitX(),
        Vec3::UnitY(),
        -Vec3::UnitY(),
        Vec3::UnitZ(),
        -Vec3::UnitZ(),
    };
    static const Vec3 up[3] = {
        Vec3::UnitY(),
        Vec3::UnitX(),
        Vec3::UnitY(),
    };
    static const Vec3 rotateUp[3] = {
        Vec3::UnitZ(),
        Vec3::UnitZ(),
        Vec3::UnitX(),
    };
    static const IVec3 axis[3] = {      //view space:       x正向向右，y正向向上，z负向向前
        IVec3(2, 1, 0),
        IVec3(2, 0, 1),
        IVec3(0, 1, 2),
    };
    static const IVec3 rotateAxis[3] = {
        IVec3(1, 2, 0),
        IVec3(0, 2, 1),
        IVec3(1, 0, 2),
    };

    for(int face = 0; face < 6; face++)  // 初始化每个card的视角信息，前后上下右左
    {
        bool rotate =   ((face == 0 || face == 1) && (extent.z() < extent.y())) ||
                        ((face == 2 || face == 3) && (extent.z() < extent.x())) ||
                        ((face == 4 || face == 5) && (extent.x() < extent.y()));
        IVec3 viewAxis = rotate ? rotateAxis[face / 2] : axis[face / 2];
        Vec3 viewDirection  = -direction[face];
        Vec3 viewUp         = rotate ? rotateUp[face / 2] : up[face / 2];

        MeshCardCache entry = {};
        entry.valid = true;
        entry.objectID = task.objectID;
        auto& card = entry.meshCard;
        card.viewPosition   = center.array() + extent.array() / 2 * direction[face].array();
        card.viewExtent     = Vec3(extent[viewAxis.x()], extent[viewAxis.y()], extent[viewAxis.z()]);
        card.scale          = task.scale;
        card.view           = Math::LookAt(card.viewPosition, center, viewUp);
        card.proj           = Math::Ortho(card.viewExtent.x() / 2, -card.viewExtent.x() / 2, 
                                        card.viewExtent.y() / 2, -card.viewExtent.y() / 2, 
                                        0.0, card.viewExtent.z());
        card.proj(1, 1) *= -1;		// Vulkan的NDC是y向下                                
        //card.proj = Math::Perspective(Math::ToRadians(90.0f), 1.0f, 0.01f, card.viewExtent.z());
        card.invView = card.view.inverse();
        card.invProj = card.proj.inverse();

        cache[task.meshCardID + face] = entry;
    }
}

void RenderSurfaceCacheManager::UpdateSurfaceCache()
{
    ENGINE_TIME_SCOPE(RenderSurfaceCacheManager::UpdateSurfaceCache);

    // auto camera = EngineContext::World()->GetActiveScene()->GetActiveCamera();

    // 遍历场景，获取绘制信息
    // TODO 场景的CPU端剔除
    std::vector<SurfaceCacheTask> tasks;
    auto rendererComponents = EngineContext::World()->GetActiveScene()->GetComponents<MeshRendererComponent>();     // 场景物体
    for(auto component : rendererComponents) component->CollectSurfaceCacheTask(tasks);

    // 在这里完成每帧内surface cache的主要操作，包括
    // 为新card分配空间、动态更新每个card所占cache分辨率、确定光栅化和光照计算的范围等

    // 目前的写法等同于：在物体首次加入渲染时，根据其当时的世界空间尺寸分配一次cache，之后不做尺寸更新
    // cache分辨率大致等于 每一米范围 ~ 8像素分辨率
    uint32_t rasterizeBuget = MAX_SURFACE_CACHE_RASTERIZE_SIZE * MAX_SURFACE_CACHE_RASTERIZE_SIZE;
    UVec2 padding = UVec2(SURFACE_CACHE_PADDING, SURFACE_CACHE_PADDING);
    rasterizeDraws.clear();
    for(auto& task : tasks) 
    {
        if(objectIDtoCardID.contains(task.objectID) && 
            objectIDtoCardID[task.objectID] != task.meshCardID) 
        {
            ReleaseCache(task.meshCardID);                              // ID改变？ TODO 
            objectIDtoCardID.erase(task.objectID);
        }
        if(!objectIDtoCardID.contains(task.objectID))    InitCache(task);    // 首次提交的数据，先初始化
                                                                                     
        // 1. 分配cache空间                                                 
        for(int face = 0; face < 6; face++)
        {
            auto& entry = cache[task.meshCardID + face];
            auto& range = entry.atlasRange;
            if(!entry.valid) continue;
            if(!range)
            {
                float scaleFactor = 8.0;
                UVec2 paddedExtent = UVec2(entry.meshCard.viewExtent.x() * scaleFactor, 
                                        entry.meshCard.viewExtent.y() * scaleFactor);    // 尺寸暂时就这样？ TODO
                if(paddedExtent.x() * paddedExtent.y() < 512.0)  paddedExtent *= 4;         // 适当扩大一点小物体的？   TODO
                paddedExtent += padding;

                range = atlas.Allocate(paddedExtent);
                if(!range) continue;    // 分配失败
                entry.meshCard.atlasOffset = range->offset + padding;
                entry.meshCard.atlasExtent = range->extent - padding;
            }

            EngineContext::RenderResource()->SetMeshCardInfo(entry.meshCard, task.meshCardID + face);
        }
        
        // 2. 处理光栅化任务
        if(rasterizeBuget < MIN_SURFACE_CACHE_LOD_SIZE * MIN_SURFACE_CACHE_LOD_SIZE) continue;
        for(int face = 0; face < 6; face++)
        {
            auto& entry = cache[task.meshCardID + face];
            auto& range = entry.atlasRange;
            if(!entry.valid) continue;
            if(!range) continue;
            if(entry.rasterized == true) continue; 

            uint32_t size = range->AllocatedSize();
            if(rasterizeBuget > size)
            {
                rasterizeBuget -= size;
                entry.rasterized = true;

                rasterizeDraws.emplace_back();
                rasterizeDraws.back().task = task;
                rasterizeDraws.back().task.meshCardID = task.meshCardID + face;  
                rasterizeDraws.back().atlasOffset = range->offset + padding;
                rasterizeDraws.back().atlasExtent = range->extent - padding;    
            }
        }
    }

    // 3. 处理光照计算任务
    // UE使用的是以每128*128的page为基本单位进行更新，这里直接用的是card为单位 
    EngineContext::RenderResource()->GetMeshCardReadback(readback); // 根据回读信息来做优先级队列
    std::sort(sortedID.begin() + 1,                             // 响应速度会快不少，相应的会多一些CPU开销，UE是GPU radix sort实现的
              sortedID.begin() + maxUsedMeshCardID * 6 + 1, 
              [&](const uint32_t& a, const uint32_t& b) {
        if(!cache[a].valid) return false;
        if(!cache[b].valid) return true;
        return  ((int)readback[a] - (int)cache[a].lastUpdateTick) >= 
                ((int)readback[b] - (int)cache[b].lastUpdateTick);   // lastUsed - lastUpdated
    });

    uint32_t directLightingBuget = MAX_SURFACE_CACHE_DIRECT_LIGHTING_SIZE * MAX_SURFACE_CACHE_DIRECT_LIGHTING_SIZE;
    directLightingDispatches.clear();
    for(int i = 1; i < maxUsedMeshCardID * 6 + 1; i++)
    {
        auto& entry = cache[sortedID[i]];
        if(!entry.valid) continue;

        if(directLightingBuget < SURFACE_CACHE_DIRECT_LIGHTING_TILE_SIZE * SURFACE_CACHE_DIRECT_LIGHTING_TILE_SIZE) break;
        {
            auto& range = entry.atlasRange;
            if(!range) continue;
            if(!entry.rasterized) continue;
            //if(EngineContext::GetCurretTick() - entry.lastUpdateTick < 30) continue; // 也可以简单的遍历，对超过固定帧数更新一次

            uint32_t size = range->AllocatedSize();
            if(directLightingBuget > size)
            {
                directLightingBuget -= size;
                entry.lastUpdateTick = EngineContext::GetCurretTick();

                if(entry.directLightings.size() == 0)
                {
                    UVec2 tiledOffset = entry.atlasRange->TiledOffset(SURFACE_CACHE_DIRECT_LIGHTING_TILE_SIZE);
                    UVec2 tiledExtent = entry.atlasRange->TiledExtent(SURFACE_CACHE_DIRECT_LIGHTING_TILE_SIZE);
                    for(int col = 0; col < tiledExtent.x(); col++)
                    {
                        for(int row = 0; row < tiledExtent.y(); row++)
                        {
                            entry.directLightings.emplace_back();
                            entry.directLightings.back().meshCardID = sortedID[i]; 
                            entry.directLightings.back().objectID = entry.objectID;
                            entry.directLightings.back().tileOffset = tiledOffset;
                            entry.directLightings.back().tileIndex = UVec2(col, row);   
                        }
                    }
                }
                directLightingDispatches.insert(directLightingDispatches.end(), 
                                                entry.directLightings.begin(), 
                                                entry.directLightings.end());
            }
        }
    }
}