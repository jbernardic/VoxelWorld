#include voxel.shader

void main()
{
    vec3 rayDirection = normalize(
        vertexPosition - camera_position
    );
    
    vec3 voxel_position;
    float voxel = raycast(camera_position, rayDirection, voxel_position);

    fragColor = vec4(0.0);

    if(voxel > 0.0) {
        fragColor = vec4(0.0);
        vec3 hit;
        float sun_ray_block = raycast(voxel_position-normalize(sun_direction)*1.5, -sun_direction, hit);
        if(sun_ray_block == 0.0){
            fragColor = vec4(1.0);
        }   
    }
}