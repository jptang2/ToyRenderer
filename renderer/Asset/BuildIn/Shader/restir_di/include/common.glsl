#define RESERVOIR_SIZE 1
#define NUM_NEIGHBORS 4

#include "reservoir.glsl"

struct RestirSetting
{
	int initialLightSampleCount;

	int temporalSampleCountMultiplier;
	float temporalPosThreshold;

	float spatialPosThreshold;
	float spatialNormalThreshold;
	float spatialRadius;

    uint visibilityReuse;
    uint temporalReuse;
    uint spatialReuse;
	uint unbias;
	uint bruteLighting;
	uint showWeight;
	uint showLightID;
};

//////////////////////////////////////////////////////////////////////////

layout (set = 1, binding = 0) buffer setting
{
	RestirSetting RESTIR_SETTING;
};

layout (set = 1, binding = 1) buffer Reservoirs 
{
	Reservoir RESERVOIRS[];                         // 本帧新生成的Reservoir(以及时域合并的结果)
};

layout (set = 1, binding = 2) buffer PrevFrameReservoirs 
{
	Reservoir PREV_RESERVOIRS[];                    // 前一帧的Reservoir结果
};

layout (set = 1, binding = 3) buffer ResultReservoirs 
{
	Reservoir RESULT_RESERVOIRS[];                  // 本帧Reservoir空域合并的结果
};

layout(set = 2, binding = 0, rgba8)         uniform image2D G_BUFFER_DIFFUSE_ROUGHNESS;	        // 本帧G-Buffer
layout(set = 2, binding = 1, rgba8_snorm)   uniform image2D G_BUFFER_NORMAL_METALLIC;	
layout(set = 2, binding = 2, rgba16f)       uniform image2D G_BUFFER_EMISSION;
layout(set = 2, binding = 3)   				uniform texture2D REPROJECTION_RESULT;				// 重投影结果	

layout(set = 2, binding = 4, rgba16f)       uniform image2D FINAL_COLOR;	                    // lighting的输出

//////////////////////////////////////////////////////////////////////////

vec3 EvaluatePHatFull(
	uint lightID, vec3 worldPos, vec3 normal, 
	vec3 diffuse, float roughness, float metallic) 
{
	vec3 N = normalize(normal);
	vec3 V = normalize(CAMERA.pos.xyz - worldPos);	

	return PointLighting(diffuse, roughness, metallic, 
						 vec4(worldPos, 1.0f), N, V, lightID);
}

float EvaluatePHat(
	uint lightID, vec3 worldPos, vec3 normal,  
	vec3 diffuse, float roughness, float metallic) 
{
	return 	RGBtoLuminance(EvaluatePHatFull(
				lightID, worldPos, normal, 
				diffuse, roughness, metallic));	
}