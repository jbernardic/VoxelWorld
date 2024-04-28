#include voxel.shader

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