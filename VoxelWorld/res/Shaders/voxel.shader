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
uniform vec3 sun_position = vec3(50, 80, 20);

uniform vec3 ambient_color = vec3(0.5, 0.0, 0.0);

uniform float sun_intensity = 1.0;
uniform float sun_reach = 30.0;
uniform float ambient_intensity = 1.0;



uniform vec3 grid_size;
uniform sampler3D voxelTexture;

const float VOXEL_SIZE = 1;
const int MAX_RAY_STEPS = 200;


vec4 raycast(vec3 rayPos, vec3 rayDir, out float camera_distance, out float sun_distance)
{
    rayPos = rayPos / VOXEL_SIZE;
    ivec3 mapPos = ivec3(floor(rayPos + 0.));
    vec3 deltaDist = abs(vec3(length(rayDir)) / rayDir);
    ivec3 rayStep = ivec3(sign(rayDir));
    vec3 sideDist = (sign(rayDir) * (vec3(mapPos) - rayPos) + (sign(rayDir) * 0.5) + 0.5) * deltaDist;

    for (int i = 0; i < MAX_RAY_STEPS; i++) {
        if (mapPos.x >= 0 && mapPos.x <= (grid_size.x - 1) &&
            mapPos.y >= 0 && mapPos.y <= (grid_size.y - 1) &&
            mapPos.z >= 0 && mapPos.z <= (grid_size.z - 1)) {
            vec4 val = texture(voxelTexture, (vec3(mapPos) + 0.5) / vec3(grid_size));
            if (val.w == 1) {
                camera_distance = sqrt((mapPos.x - rayPos.x) * (mapPos.x - rayPos.x) +
                                (mapPos.y - rayPos.y) * (mapPos.y - rayPos.y) +
                                (mapPos.z - rayPos.z) * (mapPos.z - rayPos.z)) * VOXEL_SIZE;
                rayPos = sun_position / VOXEL_SIZE;
                sun_distance = sqrt((mapPos.x - rayPos.x) * (mapPos.x - rayPos.x) +
                                (mapPos.y - rayPos.y) * (mapPos.y - rayPos.y) +
                                (mapPos.z - rayPos.z) * (mapPos.z - rayPos.z)) * VOXEL_SIZE;
                return val;
            }
        }
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
    
    float camera_distance;
    float sun_distance;
    
    vec4 voxel_color = raycast(camera_position, rayDirection, camera_distance, sun_distance);

    if(voxel_color.w > 0.0) {
        fragColor = voxel_color;
        fragColor = mix(fragColor, vec4(ambient_color, 1.0), clamp(sun_distance/sun_reach, 1.0-sun_intensity, ambient_intensity));
    }
    else fragColor = vec4(0.0);
}