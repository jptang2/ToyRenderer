#define NUM_NEIGHBORS 4

#include "reservoir.glsl"

struct RestirGISetting
{
	int initialSampleCount;

	int temporalSampleCountMultiplier;
	float temporalPosThreshold;

	float spatialPosThreshold;
	float spatialNormalThreshold;
	float spatialRadius;

    uint visibilityReuse;
    uint temporalReuse;
    uint spatialReuse;
	uint unbias;
	uint enableSkybox;
	uint giOnly;                   
	uint showRadiance;             
};

//////////////////////////////////////////////////////////////////////////

layout (set = 1, binding = 0) buffer setting
{
	RestirGISetting RESTIR_GI_SETTING;
};

layout (set = 1, binding = 1) buffer Reservoirs 
{
	GIReservoir RESERVOIRS[];                         // 本帧新生成的Reservoir(以及时域合并的结果)
};

layout (set = 1, binding = 2) buffer PrevFrameReservoirs 
{
	GIReservoir PREV_RESERVOIRS[];                    // 前一帧的Reservoir结果
};

layout (set = 1, binding = 3) buffer ResultReservoirs 
{
	GIReservoir RESULT_RESERVOIRS[];                  // 本帧Reservoir空域合并的结果
};

layout(set = 2, binding = 0, rgba8)         uniform image2D G_BUFFER_DIFFUSE_ROUGHNESS;	        // 本帧G-Buffer
layout(set = 2, binding = 1, rgba8_snorm)   uniform image2D G_BUFFER_NORMAL_METALLIC;	
layout(set = 2, binding = 2, rgba16f)       uniform image2D G_BUFFER_EMISSION;
layout(set = 2, binding = 3)   				uniform texture2D REPROJECTION_RESULT;				// 重投影结果	

layout(set = 2, binding = 4, rgba16f)       uniform image2D FINAL_COLOR;	                    // lighting的输出



float EvaluatePHat(in HitSample hitSample)
{
	return RGBtoLuminance(hitSample.outRadiance);
}