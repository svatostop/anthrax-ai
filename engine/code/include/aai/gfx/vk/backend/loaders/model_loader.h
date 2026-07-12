#pragma once
#include <complex>
#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "aai/utils/utils.h"
#include "aai/gfx/vk/model/model_types.h"

// TODO rethink loaders folder inside backend folder, confusing -- same for rq

namespace loader {
    namespace gltf {
        void load(const std::string& path, std::vector<model::types::node*>& nodes, std::vector<uint16_t>& index_buffer, std::vector<model::types::vertex>& vertex_buffer, std::vector<model::types::skin>& skins, std::vector<model::animation::base>& animations);
    }
}
