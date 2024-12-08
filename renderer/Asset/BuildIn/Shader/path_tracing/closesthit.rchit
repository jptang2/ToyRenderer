#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"
#include "common.glsl"


// Multiple importance sampling balance heuristic
float MISWeightBalanced(float Pdf, float OtherPdf)
{
	// The straightforward implementation is prone to numerical overflow and divisions by 0
	// and does not work well with +inf inputs.

	if (Pdf == OtherPdf)
	{
		// Catch potential NaNs from (0,0) and (+inf, +inf)
		return 0.5f;
	}

	// Evaluate the expression using the ratio of the smaller value to the bigger one for greater
	// numerical stability. The math would also work using the ratio of bigger to smaller value,
	// which would underflow less but would make the weights asymmetric. Underflow to 0 is not a
	// bad property to have in rendering application as it ensures more weights are exactly 0
	// which allows some evaluations to be skipped.
	if (OtherPdf < Pdf)
	{
		float x = OtherPdf / Pdf;
		return 1.0 / (1.0 + x);
	}
	else
	{
		// this form guarantees the weights add back up to one when arguments are swapped
		float x = Pdf / OtherPdf;
		return 1.0 - 1.0 / (1.0 + x);
	}
}




layout(location = 0) rayPayloadInEXT Payload RAY_PAYLOAD;
hitAttributeEXT vec2 HIT_ATTRIBS;

void main()
{
	uint objectID       = gl_InstanceCustomIndexEXT;
    uint triangleID    	= gl_PrimitiveID;
	vec3 barycentrics 	= vec3(1.0f - HIT_ATTRIBS.x - HIT_ATTRIBS.y, HIT_ATTRIBS.x, HIT_ATTRIBS.y);


    mat4 model          = FetchModel(objectID);
    uvec3 index         = FetchTriangleIndex(objectID, triangleID, barycentrics);
    vec4 pos            = FetchTrianglePos(objectID, index, barycentrics);
    vec3 worldNormal    = FetchWorldNormal(FetchTriangleNormal(objectID, index, barycentrics), model);
    vec4 worldTangent   = FetchWorldTangent(FetchTriangleTangent(objectID, index, barycentrics), model);
    vec3 color          = FetchTriangleColor(objectID, index, barycentrics); 
    vec2 texCoord       = FetchTriangleTexCoord(objectID, index, barycentrics);    
    vec4 worldPos       = model * pos; 

	Material material   = FetchMaterial(objectID);
    vec4 diffuse        = FetchDiffuse(material, texCoord);
    vec3 emission       = FetchEmission(material);
    vec3 normal         = FetchNormal(material, texCoord, worldNormal, worldTangent);
    float roughness     = FetchRoughness(material, texCoord);
    float metallic      = FetchMetallic(material, texCoord);


    // 光照参数计算 //////////////////////////////////
    
    float a2        = pow(roughness, 4) ;                       //粗糙度4次方

	vec3 N          = normalize(normal);                        //法线
	vec3 V          = normalize(-gl_WorldRayDirectionEXT);	    //视线，此处是递归的出射方向

    // 间接光照 //////////////////////////////////
    vec3 throughput = vec3(0.0f);
    float pdf;
    vec3 L;
    {
        vec2 seed = vec2(RandFloat(RAY_PAYLOAD.rand), RandFloat(RAY_PAYLOAD.rand));
        
        // vec4 newSample     = CosineSampleHemisphere(seed);                   // 余弦采样
        // vec3 L             = normalize(TangentToWorld(newSample1.xyz, N));     
        // float pdf          = newSample1.w;

        vec4 newSample      = ImportanceSampleGGX(seed, a2);                    // 重要性采样
        vec3 H              = normalize(TangentToWorld(newSample.xyz, N));      // 半程向量
        float HPDF          = newSample.w;
        float VoH           = saturate(dot(V, H));
        pdf                 = RayPDFToReflectionRayPDF(VoH, HPDF);
        L                   = normalize(2.0 * dot(V, H) * H - V);               // 采样的光源入射方向

        // 应该和这个的pdf是相同的
        // float D = D_GGX( a2,  NoH );
        // pdf = D * NoH / (4 * VoH);

        float NoL = saturate(dot(N, L));      
        vec3 radiance = vec3(1.0f) * NoL; 
        
        vec3 f_r = ResolveBRDF(diffuse.xyz, roughness, metallic, N, V, L);

        throughput += f_r * radiance;

        if(NoL <= 0.0f) pdf = 0.0f;  // 终止 
    }

    // 直接光照 //////////////////////////////////   
    // NEE
    vec3 lightColor = vec3(0.0f);
    {
        float lightSamplePdf = 1.0f / (LIGHTS.lightSetting.pointLightCnt + 1);  // 点光源 + 平行光源总计数目, 直接均匀采样吧

        if(RandFloat(RAY_PAYLOAD.rand) <= lightSamplePdf)
        {
            lightColor +=   DirectionalLighting(diffuse.xyz, roughness, metallic, worldPos, N, V) * 
                            RtDirectionalShadow(worldPos) / 
                            lightSamplePdf;
        }
        else
        {
            uint lightID = LIGHTS.lightSetting.pointLightIDs[uint(round(RandFloat(RAY_PAYLOAD.rand) * (LIGHTS.lightSetting.pointLightCnt - 1)))];	// 抽样选取点光源及其选择概率，直接用均匀分布了
            lightColor +=   PointLighting(diffuse.xyz, roughness, metallic, worldPos, N, V, lightID) *
                            RtPointShadow(lightID, worldPos) /
                            lightSamplePdf;                       // 需要考虑pdf
        }     
    }

    // 自发光 //////////////////////////////////
    lightColor += emission;



    RAY_PAYLOAD.throughput = throughput;
	RAY_PAYLOAD.lightColor = lightColor;
    RAY_PAYLOAD.distance = gl_HitTEXT; 
    RAY_PAYLOAD.pdf = pdf;
    RAY_PAYLOAD.reflectDir = L;
}
