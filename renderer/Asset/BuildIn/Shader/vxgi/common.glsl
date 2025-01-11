
layout(push_constant) uniform vxgi_setting {
	ClipmapRegion region;
} SETTING;

layout(set = 1, binding = 0, rgba32f) uniform image3D VXGI_CLIPMAP;
layout(set = 1, binding = 1) buffer VXGI_CLIPMAP_BUFFER {
	ClipmapInfo clipmapInfo;
};

struct Payload 
{
	vec3 normal;
	vec4 diffuse;
};

