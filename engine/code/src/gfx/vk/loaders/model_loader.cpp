#include "aai/gfx/vk/loaders/model_loader.h"
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


void load_skeletons(const tinygltf::Model& gltf_model, model::info& m) {
    m.skeletons.resize(gltf_model.skins.size());
    int i = 0;
    for (const tinygltf::Skin& s : gltf_model.skins) {
        m.skeletons[i].name = s.name;
        m.skeletons[i].root_node_ind = (s.skeleton >= 0) ? s.skeleton : -1;
    
        m.skeletons[i].bones.reserve(s.joints.size());
        for (int bone_ind : s.joints) {
            m.skeletons[i].bones.push_back(bone_ind);
        }
        if (s.inverseBindMatrices > -1) {
            const tinygltf::Accessor& accessor = gltf_model.accessors[s.inverseBindMatrices];
            const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buff = gltf_model.buffers[buff_view.buffer];
            m.skeletons[i].inv_matrices.resize(accessor.count);
            memcpy(m.skeletons[i].inv_matrices.data(), &buff.data[accessor.byteOffset + buff_view.byteOffset], accessor.count * sizeof(glm::mat4));
        }
        i++;
    }
}

void load_animations(const tinygltf::Model& gltf_model, model::info& m) {
    m.clips.resize(gltf_model.animations.size());
    m.clip_names.resize(m.clips.size());
    int i =0;
    for (const tinygltf::Animation& anim : gltf_model.animations) {
        m.clips[i].name = anim.name;
        m.clip_names[i] = anim.name;
        m.clips[i].samplers.resize(anim.samplers.size());
        int j = 0;
        for (const tinygltf::AnimationSampler& sampler : anim.samplers) {
            model::animation::sampler& dst_anim_sampler = m.clips[i].samplers[j];
            if (sampler.interpolation == "STEP")
                dst_anim_sampler.interpolation = model::animation::sampler::interpolation_t::step;
            else if (sampler.interpolation == "CUBICSPLINE") 
                dst_anim_sampler.interpolation = model::animation::sampler::interpolation_t::cubic_spline;
            else
                dst_anim_sampler.interpolation = model::animation::sampler::interpolation_t::linear;
            {
                const tinygltf::Accessor& accessor = gltf_model.accessors[sampler.input];
                const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buff = gltf_model.buffers[buff_view.buffer];
                const void* data_ptr = &buff.data[accessor.byteOffset + buff_view.byteOffset];
                const float* buf = static_cast<const float*>(data_ptr);
                for (size_t ind = 0; ind < accessor.count; ind++) {
                    dst_anim_sampler.inputs.push_back(buf[ind]);
                }
                for (auto& input : m.clips[i].samplers[j].inputs) {
                    if (input < m.clips[i].start) {
                        m.clips[i].start = input;
                    }
                    if (input > m.clips[i].end) {
                        m.clips[i].end = input;
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
        m.clips[i].channels.resize(anim.channels.size());
        for (int j = 0; j < anim.channels.size(); j++) {
            tinygltf::AnimationChannel channel = anim.channels[j];
            model::animation::channel& dst_channel = m.clips[i].channels[j];
            if (channel.target_path == "translation") 
                dst_channel.path  = model::animation::channel::path_t::translation;
            else if (channel.target_path == "rotation")  
                dst_channel.path  = model::animation::channel::path_t::rotation;
            else if (channel.target_path == "scale")
                dst_channel.path  = model::animation::channel::path_t::scale;
            else                                      
                continue;
            dst_channel.sampler_index = channel.sampler;
            dst_channel.node_ind = channel.target_node;
        }
        i++;
    }
}


//
void load_node(const tinygltf::Node& input_node, int32_t node_ind, model::info& m, const tinygltf::Model& gltf_model)
{
    auto& n = m.nodes[node_ind];
    n.name = input_node.name;
    n.skin_id = input_node.skin;            
    if (input_node.translation.size() == 3) {
        n.default_translation = glm::make_vec3(input_node.translation.data());//glm::translate(n->matrix, glm::vec3(glm::make_vec3(input_node.translation.data())));
    }
    if (input_node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(input_node.rotation.data());
        n.default_rotation = glm::mat4(q);  
    }
    if (input_node.scale.size() == 3) {
        n.default_scale = glm::make_vec3(input_node.scale.data());//glm::scale(n->matrix, glm::vec3(glm::make_vec3(input_node.scale.data())));
    }
    if (input_node.matrix.size() == 16) {
        n.default_matrix = glm::make_mat4x4(input_node.matrix.data());
    }

    if (input_node.mesh > -1) {
        const tinygltf::Mesh mesh_data = gltf_model.meshes[input_node.mesh];
        for (const tinygltf::Primitive& gltf_primitive : mesh_data.primitives) {
            uint32_t first_ind  = static_cast<uint32_t>(m.index_buffer.size());
            uint32_t vertex_start = static_cast<uint32_t>(m.vertex_buffer.size());
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
                m.vertex_buffer.push_back(vert);
            }
            const tinygltf::Accessor& accessor = gltf_model.accessors[gltf_primitive.indices];
            const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = gltf_model.buffers[buff_view.buffer];
            index_count += static_cast<uint32_t>(accessor.count);

            switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* b = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);        
                    for (int i = 0; i < accessor.count; i++) {
                        m.index_buffer.push_back(b[i] + vertex_start);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* b = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                    for (int i = 0; i < accessor.count; i++) {
                        m.index_buffer.push_back(b[i] + vertex_start);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* b = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                    for (int i = 0; i < accessor.count; i++) {
                        m.index_buffer.push_back(b[i] + vertex_start);
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
            n.mesh.primitives.push_back(prim);
        }
    }
}
void loader::gltf::load(const std::string& path, model::info& m)
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
    m.nodes.resize(gltf_model.nodes.size());

    for (size_t i = 0; i < gltf_model.nodes.size(); ++i)
        load_node(gltf_model.nodes[i], static_cast<int32_t>(i), m, gltf_model);
    for (size_t i = 0; i < gltf_model.nodes.size(); ++i)
        for (int32_t child : gltf_model.nodes[i].children) {
            m.nodes[child].parent = static_cast<int32_t>(i);
            m.nodes[i].children.push_back(child);
        }

    if (!gltf_model.scenes.empty())
        for (int32_t r : gltf_model.scenes[0].nodes)
            m.root_nodes.push_back(r);

    load_skeletons(gltf_model, m);
    load_animations(gltf_model, m);
}

