#pragma once

#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/Mesh/Mesh.h"
#include "Core/SurfaceCache/SurfaceCache.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RenderResource/Buffer.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include <array>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 由component提交给manager的绘制SurfaceCache所需信息
struct SurfaceCacheTask 
{
    uint32_t meshCardID;    // 起始值
    uint32_t objectID;

    Vec3 scale;
    BoundingBox box;                 

    VertexBufferRef vertexBuffer;  
    IndexBufferRef indexBuffer; 
};

// 由manager提交给pass的光栅化所需信息
struct SurfaceCacheRasterizeDraw 
{
    UVec2 atlasOffset = UVec2::Zero();
	UVec2 atlasExtent = UVec2::Zero();

    SurfaceCacheTask task;   // meshCardID是特定face的
};

// 由manager提交给pass的光照所需信息
struct SurfaceCacheLightingDispatch
{
    uint32_t meshCardID;
    uint32_t objectID;
    UVec2 tileOffset = UVec2::Zero();       // 当前tile相对于整个surface cache的偏移
    UVec2 tileIndex = UVec2::Zero();        // 当前tile相对于card的偏移
}; 

class RenderSurfaceCacheManager 
{
public:
    void Init();
    void Tick(); 

    const std::vector<SurfaceCacheRasterizeDraw>& GetRasterizeDraws()               { return rasterizeDraws; }
    const std::vector<SurfaceCacheLightingDispatch>& GetDirectLightingDispatches()  { return directLightingDispatches; }

    void ReleaseCache(uint32_t meshCardID);

private:
    void UpdateSurfaceCache();

    void InitCache(const SurfaceCacheTask& task);

    struct MeshCardCache
    {
        uint32_t objectID = 0;
        bool rasterized[6] = { false };
        uint32_t lastUpdateTick[6] = { 0 };

        std::vector<SurfaceAtlasRangeRef> atlasRange = std::vector<SurfaceAtlasRangeRef>(6);
        std::vector<MeshCardInfo> meshCards = std::vector<MeshCardInfo>(6);

        std::vector<SurfaceCacheLightingDispatch> directLightings[6];
    };
    std::unordered_map<uint32_t, MeshCardCache> cache;      // 从meshCardID映射到对应的MeshCardCache

    MeshCardSampleReadBack sampleReadback;                                      // 回读的采样数信息
    std::vector<SurfaceCacheRasterizeDraw> rasterizeDraws;                      // 每帧需要进行光栅化的部分
    std::vector<SurfaceCacheLightingDispatch> directLightingDispatches;         // 每帧需要进行直接光照的部分

    SurfaceAtlas atlas;

    
};

