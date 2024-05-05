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

#include voxel.shader

in vec3 vertexPosition;
out vec4 fragColor;

uniform vec4 ambient = vec4(0.5, 0.09, 0.098, 1.0);

float cone_trace_ao(vec3 start, vec3 dir){
    float res = 0.0;
    const int STEP_COUNT = 3;
    vec3 position = start;
    for(int i = 1; i<=STEP_COUNT; ++i){
        float val = textureLod(opacityMap, position/vec3(grid_size), log2(i+3)).r;
        res+= (1.0 - res) * val * (0.05*i);
        position+=dir*1.5;
    }
    return res;
}

void main()
{
    vec3 rayDirection = normalize(
        vertexPosition - camera_position
    );

    vec3 voxel_position = vec3(0.0);
    float voxel = raycast_voxel(camera_position, rayDirection, voxel_position);
    if(voxel != 0.0) {

        vec3 norm_sun_dir = normalize(sun_direction);
        vec3 voxel_normal = normal_map[get_index(voxel_position)];
        float ao = 1-cone_trace_ao(voxel_position+voxel_normal*2.0, voxel_normal);

        //diffuse lightning
        float diff = max(dot(voxel_normal, -norm_sun_dir), 0.0);
        vec4 diffuse =  diff * vec4(1.0);

        //+voxel_normal-norm_sun_dir*2.8 to fix some shadow issues
        vec3 hit;
        float shadow_voxel = raycast_voxel(voxel_position+voxel_normal-norm_sun_dir*2.8, -norm_sun_dir, hit);
        if(shadow_voxel == 0.0){
            fragColor = (ambient*ao+diffuse)*get_voxel_color(voxel);
        }
        else {
            fragColor = ambient*get_voxel_color(voxel)*ao;
        }
    }
    else fragColor = vec4(0.0);
}