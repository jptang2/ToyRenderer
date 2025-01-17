#ifndef RAY_QUERY_GLSL
#define RAY_QUERY_GLSL

#if(ENABLE_RAY_TRACING != 0)

bool RayQueryVisibility(vec3 from, vec3 to) 
{
	float tmin = MIN_RAY_TRACING_DISTANCE;
    //float tmax = MAX_RAY_TRACING_DISTANCE;  
	float tmax = length(to - from);
	vec3 dir = normalize(to - from);

    rayQueryEXT query;
    rayQueryInitializeEXT(
        query, 
        TLAS, 
        gl_RayFlagsTerminateOnFirstHitEXT, 
        0xFF, 
        from, 
        tmin, 
        dir, 
        tmax);

    rayQueryProceedEXT(query);

    float dist = tmax;
    if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT)
    {
        //dist = rayQueryGetIntersectionTEXT(query, true);
        return true;
    }

    // return dist < tmax;
    return false;
}

bool RayQueryHitObject(
    vec3 from, vec3 to, 
    out uint objectID, 
    out vec4 worldPos, 
    out vec3 worldNormal) 
{
	float tmin = MIN_RAY_TRACING_DISTANCE;
    //float tmax = MAX_RAY_TRACING_DISTANCE;  
	float tmax = length(to - from);
	vec3 dir = normalize(to - from);

    rayQueryEXT query;
    rayQueryInitializeEXT(
        query, 
        TLAS, 
        gl_RayFlagsTerminateOnFirstHitEXT, 
        0xFF, 
        from, 
        tmin, 
        dir, 
        tmax);

    rayQueryProceedEXT(query);

    if (rayQueryGetIntersectionTypeEXT(query, true) != gl_RayQueryCommittedIntersectionNoneEXT)
    {

        vec2 hitAttribute   = rayQueryGetIntersectionBarycentricsEXT(query, true);
        objectID            = rayQueryGetIntersectionInstanceCustomIndexEXT(query, true);
        uint triangleID    	= rayQueryGetIntersectionPrimitiveIndexEXT(query, true);
        vec3 barycentrics   = vec3(1.0f - hitAttribute.x - hitAttribute.y, hitAttribute.x, hitAttribute.y);

        mat4 model          = FetchModel(objectID);
        uvec3 index         = FetchTriangleIndex(objectID, triangleID, barycentrics);
        vec4 pos            = FetchTrianglePos(objectID, index, barycentrics);
        worldNormal         = FetchWorldNormal(FetchTriangleNormal(objectID, index, barycentrics), model);
        worldPos            = model * pos;

        return true;
    }

    return false;
}

#endif

#endif