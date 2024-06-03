#include "ModelAsset.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <set>

static glm::mat4 convertTransform(std::variant<fastgltf::TRS, fastgltf::Node::TransformMatrix> variant)
{
    if (std::holds_alternative<fastgltf::TRS>(variant))
    {
        auto& transform = std::get<fastgltf::TRS>(variant);
        glm::vec3 tl(transform.translation[0], transform.translation[1],
            transform.translation[2]);
        glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
            transform.rotation[2]);
        glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

        glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
        glm::mat4 rm = glm::toMat4(rot);
        glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);
        return tm * rm * sm;
    }
    else
    {
        auto& transform = std::get<fastgltf::Node::TransformMatrix>(variant);
        return glm::make_mat4(transform.data());
    }
}

static std::unique_ptr<ModelAsset::Image> loadImage(fastgltf::Asset& asset, fastgltf::Image& image)
{
    std::unique_ptr<ModelAsset::Image> imageAsset;
    int width, height, nrChannels;

    std::visit(
        fastgltf::visitor {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                                                    // local files.

                const std::string path(filePath.uri.path().begin(),
                    filePath.uri.path().end()); // Thanks C++.
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    imageAsset = std::make_unique<ModelAsset::Image>(data, vk::Format::eR8G8B8A8Unorm, imagesize);
                }
            },
            [&](fastgltf::sources::Array& arr) {
                unsigned char* data = stbi_load_from_memory(arr.bytes.data(), static_cast<int>(arr.bytes.size()),
                    &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    imageAsset = std::make_unique<ModelAsset::Image>(data, vk::Format::eR8G8B8A8Unorm, imagesize);
                }
            },
            [&](fastgltf::sources::BufferView& view) {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];
                std::visit(fastgltf::visitor {
                               [](auto& arg) {},
                               [&](fastgltf::sources::Array& arr) {
                                    auto a = arr.bytes.data();
                                    for (int i = 0; i < arr.bytes.size(); i++)
                                    {
                                        int a = arr.bytes.data()[i];
                                    }
                                    unsigned char* data = stbi_load_from_memory(arr.bytes.data() + bufferView.byteOffset, static_cast<int>(arr.bytes.size()),
                                        &width, &height, &nrChannels, 4);
                                    if (data) {
                                        VkExtent3D imagesize;
                                        imagesize.width = width;
                                        imagesize.height = height;
                                        imagesize.depth = 1;

                                        imageAsset = std::make_unique<ModelAsset::Image>(data, vk::Format::eR8G8B8A8Unorm, imagesize);
                                    }
                               } },
                    buffer.data);
            },
        },
        image.data);
    
    if (!imageAsset)
    {
        imageAsset = std::make_unique<ModelAsset::Image>();
        imageAsset->size = vk::Extent3D(1, 1, 1);
        imageAsset->format = vk::Format::eR8G8B8A8Unorm;
        imageAsset->data = (unsigned char*) & ModelAsset::ErrorColor;
        imageAsset->error = true;
    }

    return imageAsset;
}


static void processNode(int current, std::optional<uint32_t> skeletonParent, std::vector<ModelAsset::Node>& nodes, std::unordered_map<int, int>& joints,
    std::unordered_map<int, ModelAsset::Node>& skeleton, glm::mat4 transform = glm::mat4(1.0))
{
    nodes[current].globalTransform = transform * nodes[current].localTransform;

    if (joints.find(current) != joints.end())
    {
        skeleton[current] = nodes[current];
        skeleton[current].parent = skeletonParent;
        skeletonParent = current;
    }

    for (int i = 0; i < nodes[current].children.size(); i++)
    {
        int childIndex = nodes[current].children[i];
        processNode(childIndex, skeletonParent, nodes, joints, skeleton, nodes[current].globalTransform);
    }
}

ModelAsset Asset::LoadModelGLTF(std::filesystem::path filePath)
{
    std::cout << "Loading GLTF: " << filePath << std::endl;

    fastgltf::GltfDataBuffer data;
    try
    {
        data.loadFromFile(filePath);
    }
    catch (...)
    {
        throw std::runtime_error("Couldn't load: " + filePath.string());
    }
    

    constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
        | fastgltf::Options::LoadExternalBuffers;

    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    auto load = parser.loadGltfBinary(&data, filePath.parent_path(), gltfOptions);
    if (load)
    {
        gltf = std::move(load.get());
    }
    else
    {
        std::cout << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
        return {};
    }

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::vector<VertexBone> bones;
    std::vector<ModelAsset::Surface> surfaces;

    std::unordered_map<int, ModelAsset::Node> skeletonNodeMap;
    std::vector<ModelAsset::Node> skeletonNodes;

    ModelAsset asset;

    if (gltf.skins.size() > 0)
    {
        std::vector<ModelAsset::Node> nodes(gltf.nodes.size());
        std::unordered_map<int, int> jointMap;

        for (int i = 0; i < gltf.skins[0].joints.size(); i++)
        {
            jointMap[gltf.skins[0].joints[i]] = i;
        }

        for (int i = 0; i < gltf.nodes.size(); i++)
        {
            nodes[i].localTransform = convertTransform(gltf.nodes[i].transform);
            for (auto& childIndex : gltf.nodes[i].children)
            {
                nodes[i].children.push_back(childIndex);
                nodes[childIndex].parent = i;
            }
        }

        for (int i = 0; i < gltf.nodes.size(); i++)
        {
            if(!nodes[i].parent.has_value())
                processNode(i, {}, nodes, jointMap, skeletonNodeMap);
        }

        //convert skeleton map to skeleton vector 
        // based on join indices
        skeletonNodes.resize(jointMap.size());
        for (const auto& p : skeletonNodeMap)
        {
            int index = jointMap[p.first];
            skeletonNodes[index] = p.second;

            std::optional<uint32_t> parent = p.second.parent;
            if (parent.has_value())
            {
                skeletonNodes[index].parent = jointMap[parent.value()];
            }
            for (int i = 0; i<skeletonNodes[index].children.size(); i++)
            {
                skeletonNodes[index].children[i] = jointMap[skeletonNodes[index].children[i]];
            }
        }
        

        auto& inverseBindMatAccessor = gltf.accessors[gltf.skins[0].inverseBindMatrices.value()];
        uint32_t index = 0;
        fastgltf::iterateAccessor<glm::mat4>(gltf, inverseBindMatAccessor,
            [&](glm::mat4 mat) {
            auto& node = skeletonNodes[index++];
            node.inverseBindMatrix = mat;
        });
    }
    // load all textures
    for (fastgltf::Image& image : gltf.images)
    {
        std::unique_ptr<ModelAsset::Image> img = loadImage(gltf, image);
        asset.Textures.push_back(std::move(img));
    }

    for (auto& mesh : gltf.meshes)
    {
        vertices.clear();
        indices.clear();
        bones.clear();
        surfaces.clear();

        for (auto&& p : mesh.primitives)
        {
            ModelAsset::Surface newSurface;
            newSurface.firstIndex = (uint32_t)indices.size();
            newSurface.indexCount = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                    indices.push_back(idx + initial_vtx);
                });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + posAccessor.count);
                bones.resize(vertices.size());
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                    Vertex newvtx;
                    newvtx.position = v;
                    newvtx.normal = { 1, 0, 0 };
                    newvtx.uv_x = 0;
                    newvtx.uv_y = 0;
                    vertices[initial_vtx + index] = newvtx;
                });
            }

            // load joints
            auto joints = p.findAttribute("JOINTS_0");
            if (joints != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*joints).second],
                    [&](glm::vec4 v, size_t index) {
                    bones[initial_vtx + index].joint = v;
                });
            }

            // load weights
            auto weights = p.findAttribute("WEIGHTS_0");
            if (weights != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*weights).second],
                    [&](glm::vec4 v, size_t index) {
                    bones[initial_vtx + index].weight = v;
                });
            }


            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end())
            {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) {
                    vertices[initial_vtx + index].normal = v;
                });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) {
                    vertices[initial_vtx + index].uv_x = v.x;
                    vertices[initial_vtx + index].uv_y = v.y;
                });
            }

            surfaces.push_back(newSurface);
        }
        ModelAsset::Mesh mesh;
        mesh.Indices = std::move(indices);
        mesh.VertexBones = std::move(bones);
        mesh.Surfaces = std::move(surfaces);
        mesh.Vertices = std::move(vertices);
        asset.Meshes.emplace_back(std::move(mesh));
    }
    asset.Skeleton = std::move(skeletonNodes);

    return asset;
}

ModelAsset::Image::~Image()
{
    if(!error)
        stbi_image_free(data);
}