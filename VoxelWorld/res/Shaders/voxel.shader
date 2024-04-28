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
uniform vec3 sun_direction = vec3(-2.0, -2.0, -2.0);
uniform vec3 ambient_color = vec3(0.5);


uniform vec3 grid_size;
layout(binding = 0) uniform sampler3D voxelTexture;
layout(binding = 1) uniform sampler1D voxelPalette;

const int MAX_RAY_STEPS = 200;

float distance(vec3 a, vec3 b){
    return  sqrt((a.x - b.x) * (a.x - b.x) +
                    (a.y - b.y) * (a.y - b.y) +
                    (a.z - b.z) * (a.z - b.z));
}

float get_voxel(vec3 coord){
    return texture(voxelTexture, (coord + 0.5) / vec3(grid_size)).r;
}

vec4 raycast(vec3 rayPos, vec3 rayDir, out vec3 hitPosition)
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
            float texel = get_voxel(vec3(mapPos));
            vec4 val = texture(voxelPalette, texel);
            if (val.w == 1) {
                hitPosition = vec3(mapPos)+0.5;
                return val;
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
    return vec4(0.0);
}

void main()
{
    vec3 rayDirection = normalize(
        vertexPosition - camera_position
    );
    
    vec3 voxel_position;
    vec4 voxel_color = raycast(camera_position, rayDirection, voxel_position);

    if(voxel_color.w > 0.0) {
        fragColor = vec4(ambient_color, 1.0)*voxel_color;

        vec3 hit;
        vec4 t = raycast(voxel_position-normalize(sun_direction)*1.5, -sun_direction, hit);
        vec3 diffuse = vec3(0.0);
        if(t.w == 0.0){
            fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        
    }
    else fragColor = vec4(0.0);
}