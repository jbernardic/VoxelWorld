#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : require

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
}; 

struct VertexBone{
	vec4 joint;
	vec4 weight;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout(buffer_reference, std430) readonly buffer VertexBoneBuffer { 
	VertexBone bones[];
};

layout(buffer_reference, std430) readonly buffer JointMatrixBuffer{ 
	mat4 joints[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 render_matrix;
	VertexBuffer vertexBuffer;
	VertexBoneBuffer vertexBoneBuffer;
	JointMatrixBuffer jointMatrixBuffer;
} PushConstants;

void main() 
{	
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	VertexBone bone = PushConstants.vertexBoneBuffer.bones[gl_VertexIndex];

	mat4 skinMat =
    bone.weight.x * PushConstants.jointMatrixBuffer.joints[int(bone.joint.x)] +
    bone.weight.y * PushConstants.jointMatrixBuffer.joints[int(bone.joint.y)] +
    bone.weight.z * PushConstants.jointMatrixBuffer.joints[int(bone.joint.z)] +
    bone.weight.w * PushConstants.jointMatrixBuffer.joints[int(bone.joint.w)];

	vec4 position = skinMat * vec4(v.position, 1.0f);
	gl_Position = PushConstants.render_matrix * position;
	

	outColor = vec3(1.0);
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
}