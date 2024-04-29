#include voxel.shader

void main()
{
    vec3 rayDirection = normalize(
        vertexPosition - camera_position
    );

    vec3 voxel_position = vec3(0.0);
    float voxel = raycast(camera_position, rayDirection, voxel_position);
    if(voxel != 0.0) {

        vec3 hit;
        //1.6 for shadow rays to pass through themselves and neighbour voxel, since I cast rays from middle. 
        float sun_ray_block = raycast(voxel_position-normalize(sun_direction)*1.6, -sun_direction, hit);
        if(sun_ray_block == 0.0){

            fragColor = vec4(1.0);
        }
        else fragColor = get_voxel_color(voxel);
    }
    else fragColor = vec4(0.1);
}