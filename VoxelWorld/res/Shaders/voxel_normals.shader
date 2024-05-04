#shader compute
#version 450 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

#include voxel.shader

vec3 calculate_normal(vec3 coord, int sample_size){
    vec3 normal = vec3(0.0);
    //skip non visible voxels
    int cnt = 0;
    for(int x = -1; x<=1; x++){
        for(int y = -1; y<=1; y++){
            for(int z = -1; z<=1; z++){
                if(get_voxel(coord+vec3(x, y, z)) > 0.0){
                    cnt++;
                }
            }
        }
    }
   // if(cnt == 27 || cnt<=1) return normal;

    for(int x = -sample_size+1; x<sample_size; x++){
        for(int y = -sample_size+1; y<sample_size; y++){
            for(int z = -sample_size+1; z<sample_size; z++){
                if(get_voxel(coord+vec3(x, y, z)) > 0.0){
                    normal+=vec3(x, y, z);
                }
            }
        }
    }

    return -normalize(normal);
}


uniform vec3 work_position;

void main() {

    vec3 work_size = gl_WorkGroupSize*gl_NumWorkGroups;

    vec3 work_coord = floor(work_position) + gl_GlobalInvocationID.xyz 
    - work_size/2;

    if(work_coord.x < 0 || work_coord.y < 0 || work_coord.z < 0 || 
        work_coord.x >= grid_size.x || work_coord.y >= grid_size.y || work_coord.z >= grid_size.z){
        return;
    }
    
    int index = get_index(work_coord);
    vec3 normal = calculate_normal(work_coord, 6);
    normal_map[index] = normal;
    
    // if(normal != vec3(0.0)){
    //     //calculate AO

    // }
}