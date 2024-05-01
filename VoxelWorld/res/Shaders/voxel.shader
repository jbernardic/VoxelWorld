layout(binding = 0) uniform sampler3D voxelTexture;
layout(binding = 1) uniform sampler1D voxelPalette;

layout(std430, binding = 2) buffer normal_map_storage
{
    vec3 normal_map[];
};

layout (std140, binding = 3) uniform voxel_ub
{
    uniform vec3 camera_position;
    uniform vec3 sun_direction;
    uniform ivec3 grid_size;
};

int get_index(vec3 coord){
    ivec3 mapPos = ivec3(floor(coord));
    return mapPos.x + mapPos.y * grid_size.x + mapPos.z * grid_size.x * grid_size.y;
}


float get_voxel(vec3 coord){
    if(coord.x < 0 || coord.y < 0 || coord.z < 0 || 
        coord.x >= grid_size.x || coord.y >= grid_size.y || coord.z >= grid_size.z){
        return 0.0;
    }
    return texture(voxelTexture, coord / vec3(grid_size)).r;
}

vec4 get_voxel_color(float voxel){
    return texture(voxelPalette, voxel);
}

const int MAX_RAY_STEPS = 1000;

float raycast(vec3 rayPos, vec3 rayDir, out vec3 hitPosition)
{
    ivec3 mapPos = ivec3(floor(rayPos + 0.));
    vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);
    ivec3 rayStep = ivec3(sign(rayDir));
    vec3 sideDist = (sign(rayDir) * (vec3(mapPos) - rayPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist;
    vec3 prev_mapPos = vec3(mapPos);
    vec3 prev_sideDist = sideDist;

    for (int i = 0; i < MAX_RAY_STEPS; i++) {
        if (mapPos.x >= 0 && mapPos.x <= (grid_size.x - 1) &&
            mapPos.y >= 0 && mapPos.y <= (grid_size.y - 1) &&
            mapPos.z >= 0 && mapPos.z <= (grid_size.z - 1)) {
            hitPosition = vec3(mapPos)+0.5;
            float voxel = get_voxel(hitPosition);
            if(voxel > 0.0){
                return voxel;
            }
        }
        prev_mapPos = vec3(mapPos);
        prev_sideDist = sideDist;
        if (sideDist.x < sideDist.y) {
            if (sideDist.x < sideDist.z) {
                sideDist.x += deltaDist.x;
                mapPos.x += rayStep.x;
            } else {
                sideDist.z += deltaDist.z;
                mapPos.z += rayStep.z;
            }
        } else {
            if (sideDist.y < sideDist.z) {
                sideDist.y += deltaDist.y;
                mapPos.y += rayStep.y;
            } else {
                sideDist.z += deltaDist.z;
                mapPos.z += rayStep.z;
            }
        }
    }
    return 0.0;
}