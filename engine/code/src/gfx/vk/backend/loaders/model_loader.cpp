#include "aai/gfx/vk/backend/loaders/model_loader.h"
#include <cstring>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
// #define TINYGLTF_NO_STB_IMAGE_WRITE
// #define TINYGLTF_NO_STB_IMAGE
// #define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

model::types::node* find_node(uint32_t ind, model::types::node* parent) {
    model::types::node* n = nullptr;
    if (parent->index == ind) {
        return parent;
    } 
    for (auto& c : parent->children) {
        n = find_node(ind, c);
        if (n)
            break;
    }
    return n;
}
model::types::node* node_from_index(uint32_t ind, std::vector<model::types::node*>& nodes) {
    model::types::node* n = nullptr;
    for (auto& d : nodes) {
        n = find_node(ind, d);
        if (n)
            break;
    }
    return n;
}

void load_skins(const tinygltf::Model& gltf_model, std::vector<model::types::skin>& skins, std::vector<model::types::node*>& nodes) {
    skins.resize(gltf_model.skins.size());
    int i = 0;
    for (const tinygltf::Skin& s : gltf_model.skins) {
        skins[i].name = s.name;
        skins[i].skeleton_root = node_from_index(s.skeleton, nodes);

        for (int bone_ind : s.joints) {
            model::types::node* n = node_from_index(bone_ind, nodes);
            if (n)
                skins[i].bones.push_back(n);
        }
        if (s.inverseBindMatrices > -1) {
            const tinygltf::Accessor& accessor = gltf_model.accessors[s.inverseBindMatrices];
            const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buff = gltf_model.buffers[buff_view.buffer];
            skins[i].inv_matrices.resize(accessor.count);
            memcpy(skins[i].inv_matrices.data(), &buff.data[accessor.byteOffset + buff_view.byteOffset], accessor.count * sizeof(glm::mat4));
        }
        i++;
    }
}

void load_animations(const tinygltf::Model& gltf_model, std::vector<model::animation::base>& animations, std::vector<model::types::node*>& nodes) {
    animations.resize(gltf_model.animations.size());
    int i =0;
    for (const tinygltf::Animation& anim : gltf_model.animations) {
        animations[i].name = anim.name;
        animations[i].samplers.resize(anim.samplers.size());
        int j = 0;
        for (const tinygltf::AnimationSampler& sampler : anim.samplers) {
            model::animation::sampler& dst_anim_sampler = animations[i].samplers[j];
            dst_anim_sampler.interpolation = sampler.interpolation;
            
            {
                const tinygltf::Accessor& accessor = gltf_model.accessors[sampler.input];
                const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buff = gltf_model.buffers[buff_view.buffer];
                const void* data_ptr = &buff.data[accessor.byteOffset + buff_view.byteOffset];
                const float* buf = static_cast<const float*>(data_ptr);
                for (size_t ind = 0; ind < accessor.count; ind++) {
                    dst_anim_sampler.inputs.push_back(buf[ind]);
                }
                for (auto& input : animations[i].samplers[j].inputs) {
                    if (input < animations[i].start) {
                        animations[i].start = input;
                    }
                    if (input > animations[i].end) {
                        animations[i].end = input;
                    }
                }
            }
            {
                const tinygltf::Accessor& accessor = gltf_model.accessors[sampler.output];
                const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buff = gltf_model.buffers[buff_view.buffer];
                const void* data_ptr = &buff.data[accessor.byteOffset + buff_view.byteOffset];
                switch(accessor.type) {
                    case TINYGLTF_TYPE_VEC3: {
                        const glm::vec3* b = static_cast<const glm::vec3*>(data_ptr);
                        for (size_t ind = 0; ind < accessor.count; ind++) {
                            dst_anim_sampler.outputs.push_back(glm::vec4(b[ind], 0.0f));
                        }
                        break;
                    }
                    case TINYGLTF_TYPE_VEC4: {
                        const glm::vec4* b = static_cast<const glm::vec4*>(data_ptr);
                        for (size_t ind = 0; ind < accessor.count; ind++) {
                            dst_anim_sampler.outputs.push_back(b[ind]);
                        }
                        break;
                    }
                    default:
                        header_utils::ASSERT(true, "gltf error: failed to load gltf animation model");
                        break;
                }
            }
            j++;
        }
        animations[i].channels.resize(anim.channels.size());
        for (int j = 0; j < anim.channels.size(); j++) {
            tinygltf::AnimationChannel channel = anim.channels[j];
            model::animation::channel& dst_channel = animations[i].channels[j];
            dst_channel.path = channel.target_path;
            dst_channel.sampler_index = channel.sampler;
            dst_channel.n = node_from_index(channel.target_node, nodes);
        }
        i++;
    }
}

glm::mat4 get_node_matrix(model::types::node* n) {
    glm::mat4 m = n->get_local_matrix();
    model::types::node* parent = n->parent;
    while (parent) {
        m = parent->get_local_matrix() * m;
        parent = parent->parent; 
    }
    return m;
}

void loader::gltf::update_bones(model::types::node* n, std::vector<model::types::skin>& skins) {
    if (n->skin_id > -1) {
        glm::mat4 inv_transofrm = glm::inverse(get_node_matrix(n));
        model::types::skin& s = skins[n->skin_id];
        size_t num_bone = s.bones.size();
        s.fin_bone_transforms.resize(num_bone);
        int i = 0;
        for (glm::mat4& m : s.fin_bone_transforms) {
            m = get_node_matrix(s.bones[i]) * s.inv_matrices[i];
            m = inv_transofrm * m;
            i++;
        }
    }
    for (auto& child : n->children) {
        update_bones(child, skins);
    }
}

void load_node(const tinygltf::Node& input_node,  uint32_t node_ind, std::vector<model::types::node*>& nodes, const tinygltf::Model& gltf_model, model::types::node* parent, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer)
        {
            model::types::node* n = new model::types::node{};
            n->name = input_node.name;
            n->parent = parent;
            n->index = node_ind;
            n->skin_id = input_node.skin;
            n->matrix = glm::mat4(1.0f);
            if (input_node.translation.size() == 3) {
                n->matrix = glm::translate(n->matrix, glm::vec3(glm::make_vec3(input_node.translation.data())));
            }
            if (input_node.rotation.size() == 4) {
                glm::quat q = glm::make_quat(input_node.rotation.data());
                n->matrix *= glm::mat4(q);  
            }
            if (input_node.scale.size() == 3) {
                n->matrix = glm::scale(n->matrix, glm::vec3(glm::make_vec3(input_node.scale.data())));
            }
            if (input_node.matrix.size() == 16) {
                n->matrix = glm::make_mat4x4(input_node.matrix.data());
            }

            if (!input_node.children.empty()) {
                for (const int& v : input_node.children) {
                    load_node(gltf_model.nodes[v], v, nodes,gltf_model, n, index_buffer, vertex_buffer);
                }
            }

            if (input_node.mesh > -1) {
                const tinygltf::Mesh mesh_data = gltf_model.meshes[input_node.mesh];
                for (const tinygltf::Primitive& gltf_primitive : mesh_data.primitives) {
                    uint32_t first_ind  = static_cast<uint32_t>(index_buffer.size());
                    uint32_t vertex_start = static_cast<uint32_t>(vertex_buffer.size());
                    uint32_t index_count = 0;
                    uint32_t vertex_count = 0;
                    const float* pos_buffer = nullptr;
                    const float* normal_buffer = nullptr;
                    const float* tang_buffer = nullptr;
                    const float* coord_buffer = nullptr;
                    const uint16_t* bones_ids_buffer = nullptr;
                    const float* bones_weights_buffer = nullptr;

                    bool is_skin = false;

                    const auto& pos_it = gltf_primitive.attributes.find("POSITION");
                    if (pos_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[pos_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        pos_buffer = reinterpret_cast<const float*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                        vertex_count = accessor.count;
                    }
                    const auto& normal_it = gltf_primitive.attributes.find("NORMAL");
                    if (normal_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[normal_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        normal_buffer = reinterpret_cast<const float*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                    }
                    const auto& coord_it = gltf_primitive.attributes.find("TEXCOORD_0");
                    if (coord_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[coord_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        coord_buffer = reinterpret_cast<const float*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                    }
                    const auto& tanget_it = gltf_primitive.attributes.find("TANGENT");
                    if (tanget_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[tanget_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        tang_buffer = reinterpret_cast<const float*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                    }
                    const auto& bone_id_it = gltf_primitive.attributes.find("JOINTS_0");
                    if (bone_id_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[bone_id_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        bones_ids_buffer = reinterpret_cast<const uint16_t*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                    }
                    const auto& bone_weights_it = gltf_primitive.attributes.find("WEIGHTS_0");
                    if (bone_weights_it != gltf_primitive.attributes.end()) {
                        const tinygltf::Accessor& accessor = gltf_model.accessors[bone_weights_it->second];
                        const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                        bones_weights_buffer = reinterpret_cast<const float*>(&(gltf_model.buffers[buff_view.buffer].data[accessor.byteOffset + buff_view.byteOffset]));
                    }
                    is_skin = bones_weights_buffer && bones_ids_buffer;

                    for (size_t v = 0; v < vertex_count; v++) {
                        model::types::vertex vert{};
                        vert.position = glm::vec4(glm::make_vec3(&pos_buffer[v * 3]), 1.0f);
                        vert.normal = glm::normalize(glm::vec3(normal_buffer ? glm::make_vec3(&normal_buffer[v * 3]) : glm::vec3(0.0f)));
                        vert.uv = coord_buffer ? glm::make_vec2(&coord_buffer[v * 2]) : glm::vec2(0.0f);
                        vert.color = glm::vec3(1.0f);
                        vert.bone_ids = is_skin ? glm::ivec4(glm::make_vec4(&bones_ids_buffer[v * 4])) : glm::ivec4(-1);
                        vert.weights = is_skin ? glm::make_vec4(&bones_weights_buffer[v * 4]) : glm::vec4(0.0);
                        vertex_buffer.push_back(vert);
                    }
                    const tinygltf::Accessor& accessor = gltf_model.accessors[gltf_primitive.indices];
                    const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = gltf_model.buffers[buff_view.buffer];
                    index_count += static_cast<uint32_t>(accessor.count);

                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t* b = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);        
                            for (int i = 0; i < accessor.count; i++) {
                                index_buffer.push_back(b[i] + vertex_start);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t* b = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                            for (int i = 0; i < accessor.count; i++) {
                                index_buffer.push_back(b[i] + vertex_start);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t* b = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                            for (int i = 0; i < accessor.count; i++) {
                                index_buffer.push_back(b[i] + vertex_start);
                            }
                            break;
                        }
                        default:
                            header_utils::ASSERT(true, "gltf_loader: index type not supprted");
                            break;
                    }
                    model::types::primitive prim{};
                    prim.first_index = first_ind;
                    prim.index_count = index_count;
                    prim.material_index = -1;
                    n->mesh.primitives.push_back(prim);
                }
            }
            if (parent) {
                parent->children.push_back(n);
            }
            else {
                nodes.push_back(n);
            }
        }
void loader::gltf::load(const std::string& path, std::vector<model::types::node*>& nodes, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer, std::vector<model::types::skin>& skins, std::vector<model::animation::base>& animations)
        {
            tinygltf::Model gltf_model;
            tinygltf::TinyGLTF loader;
            std::string err, warn;
            bool ret = false;
            std::string extension = path.substr(path.find_last_of(".") + 1);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension == "glb") {
                ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, path);
            } 
            else if (extension == "gltf") {
                ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, path);
            } 
            else {
                err = "Unsupported file extension: " + extension + ". Expected .gltf or .glb";
            }
            header_utils::ASSERT(!warn.empty(), "gltf warning: " + warn); 
            header_utils::ASSERT(!err.empty(), "gltf error: " + err); 
            header_utils::ASSERT(!ret, "gltf error: failed to load gltf model");

            //load_images
            //load_materials
            //load_textures
            const tinygltf::Scene& scene = gltf_model.scenes[0];
            for (size_t i = 0; i < scene.nodes.size(); i++) {
                const tinygltf::Node node = gltf_model.nodes[scene.nodes[i]];
                load_node(node, scene.nodes[i], nodes, gltf_model, nullptr, index_buffer, vertex_buffer);
            }

            load_skins(gltf_model, skins, nodes);
            load_animations(gltf_model, animations, nodes);
            for (auto& n : nodes) {
                update_bones(n, skins);
            }

        }

