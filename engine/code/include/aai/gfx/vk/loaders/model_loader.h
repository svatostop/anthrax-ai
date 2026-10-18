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
        void load(const std::string& path, model::info& m);
    }
}
