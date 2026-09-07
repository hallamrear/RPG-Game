//Using an include guard as this is also used by the hlsl files.
#ifndef __DEFINES_H__
#define __DEFINES_H__

#define MAX_NUM_ENTITIES 1024
#define MAX_LOADABLE_TEXTURES 128
#define MAX_TEXTURES_PER_SHADER 5
#define TARGET_FPS_FRAMETIME_FLOAT (1.0f / 60.0f)
#define MAX_LIGHT_COUNT 16

#define DEFAULT_NEAR_PLANE 0.1f
#define DEFAULT_FAR_PLANE 10000.0f

#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2

#endif //__DEFINES_H__ 