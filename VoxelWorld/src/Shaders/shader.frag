#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSamplers[];

void main() {
    outColor = vec4(texture(texSamplers[0], fragTexCoord).xyz, 1.0);
}