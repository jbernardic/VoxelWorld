#shader vertex
#version 450 core
layout (location = 0) in vec3 VERTEX;

uniform mat4 camera_view;
uniform mat4 camera_projection;
uniform mat4 model = mat4(1.0);

out vec3 vertexPosition;


void main()
{
    gl_Position = camera_projection*camera_view*model*vec4(VERTEX, 1.0);
	vertexPosition = VERTEX;
}

#shader fragment
#version 450 core

in vec3 vertexPosition;
out vec4 fragColor;

uniform vec3 camera_position;
uniform vec3 sun_direction = vec3(-1.0, -1.0, -1.0);
uniform ivec3 grid_size;

layout(binding = 0) uniform sampler3D voxelTexture;
layout(binding = 1) uniform sampler1D voxelPalette;

layout(std430, binding = 0) buffer test
{
    float light_map[];
};

float get_light(vec3 coord){
    ivec3 mapPos = ivec3(floor(coord));
    return light_map[mapPos.x + mapPos.y * grid_size.x + mapPos.z * grid_size.x * grid_size.y];
}

void set_light(vec3 coord, float val){
    ivec3 mapPos = ivec3(floor(coord));
    light_map[mapPos.x + mapPos.y * grid_size.x + mapPos.z * grid_size.x * grid_size.y] = val;
}



float get_voxel(vec3 coord){
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