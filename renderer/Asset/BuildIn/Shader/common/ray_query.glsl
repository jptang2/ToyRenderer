#ifndef RAY_QUERY_GLSL
#define RAY_QUERY_GLSL

#if(ENABLE_RAY_TRACING != 0)

bool VisibilityTest(vec3 from, vec3 to) 
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

#endif

#endif