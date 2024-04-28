#include voxel.shader


layout(std430, binding = 3) buffer test
{
    bool data_SSBO[];
};

void main()
{
    vec3 rayDirection = normalize(
        vertexPosition - camera_position
    );

    vec3 voxel_position;
    float voxel = raycast(camera_position, rayDirection, voxel_position);

    if(voxel > 0.0) {

        vec3 hit;
        float sun_ray_block = raycast(voxel_position-normalize(sun_direction)*1.5, -sun_direction, hit);
        if(sun_ray_block == 0.0){
            fragColor = vec4(voxel_position, 1.0);
        }   
        else{
            fragColor = vec4(voxel_position, 0.0);
        }
    }
    else fragColor = vec4(data_SSBO[50] ? 1.0 : 0.0);
}