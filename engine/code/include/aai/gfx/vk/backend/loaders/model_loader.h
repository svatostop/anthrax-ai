#pragma once
#include <complex>
#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "aai/utils/utils.h"
#include "aai/gfx/vk/model/model_types.h"

namespace loader {
    namespace gltf {
        void load(const std::string& path, std::vector<model::types::node*>& nodes, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer);
        // {
        //     tinygltf::Model gltf_model;
        //     tinygltf::TinyGLTF loader;
        //     std::string err, warn;
        //     bool ret = false;
        //     std::string extension = path.substr(path.find_last_of(".") + 1);
        //     std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        //     if (extension == "glb") {
        //         ret = loader.LoadBinaryFromFile(&gltf_model, &err, &warn, path);
        //     } 
        //     else if (extension == "gltf") {
        //         ret = loader.LoadASCIIFromFile(&gltf_model, &err, &warn, path);
        //     } 
        //     else {
        //         err = "Unsupported file extension: " + extension + ". Expected .gltf or .glb";
        //     }
        //     header_utils::ASSERT(!warn.empty(), "gltf warning: " + warn); 
        //     header_utils::ASSERT(!err.empty(), "gltf error: " + err); 
        //     header_utils::ASSERT(!ret, "gltf error: failed to load gltf model");
        //
        //     //load_images
        //     //load_materials
        //     //load_textures
        //     const tinygltf::Scene& scene = gltf_model.scenes[0];
        //     for (size_t i = 0; i < scene.nodes.size(); i++) {
        //         const tinygltf::Node node = gltf_model.nodes[scene.nodes[i]];
        //         load_node(node, nodes, gltf_model, nullptr, index_buffer, vertex_buffer);
        //     }
        //
        // }
    }
}
