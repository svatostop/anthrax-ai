#pragma once
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"
#include "aai/utils/utils.h"
#include "aai/gfx/vk/model/model_types.h"

namespace loader {
    namespace gltf {
        void load_node(const tinygltf::Node& input_node, tinygltf::Model gltf_model, model::types::node* parent, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer)
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
                    load_node(gltf_model.nodes[v], gltf_model, n, index_buffer, vertex_buffer);
                }
            }
        }
        void load_gltf(const std::string& path, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer)
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
                load_node(node, gltf_model, nullptr, index_buffer, vertex_buffer);
            }

        }
    }
}
