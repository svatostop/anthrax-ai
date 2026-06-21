#include "aai/gfx/vk/backend/loaders/model_loader.h"
#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

void load_node(const tinygltf::Node& input_node,  std::vector<model::types::node*>& nodes, const tinygltf::Model& gltf_model, model::types::node* parent, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer)
        {
            model::types::node* n = new model::types::node{};
            n->name = input_node.name;
            n->parent = parent;

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
                    load_node(gltf_model.nodes[v], nodes,gltf_model, n, index_buffer, vertex_buffer);
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
                    for (size_t v = 0; v < vertex_count; v++) {
                        model::types::vertex vert{};
                        vert.position = glm::vec4(glm::make_vec3(&pos_buffer[v * 3]), 1.0f);
                        vert.normal = glm::normalize(glm::vec3(normal_buffer ? glm::make_vec3(&normal_buffer[v * 3]) : glm::vec3(0.0f)));
                        vert.uv = coord_buffer ? glm::make_vec2(&coord_buffer[v * 2]) : glm::vec2(0.0f);
                        vert.color = glm::vec3(1.0f);
                        vertex_buffer.push_back(vert);
                    }
                    const tinygltf::Accessor& accessor = gltf_model.accessors[gltf_primitive.indices];
                    const tinygltf::BufferView& buff_view = gltf_model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = gltf_model.buffers[buff_view.buffer];
                    index_count += static_cast<uint32_t>(accessor.count);

                    switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t* b = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                            index_buffer.resize(accessor.count);
                            memcpy(&index_buffer.data()[index_buffer.size()], &b[vertex_start], accessor.count);
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t* b = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                            // index_buffer.resize(accessor.count);
                            // memcpy(&index_buffer.data()[index_buffer.size()], &b[vertex_start], accessor.count);
                            for (int i = 0; i < accessor.count; i++) {
                                index_buffer.push_back(b[i] + vertex_start);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t* b = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + buff_view.byteOffset]);
                            index_buffer.resize(accessor.count);
                            memcpy(&index_buffer.data()[index_buffer.size()], &b[vertex_start], accessor.count);
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
void loader::gltf::load(const std::string& path, std::vector<model::types::node*>& nodes, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer)
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
                load_node(node, nodes, gltf_model, nullptr, index_buffer, vertex_buffer);
            }

        }

