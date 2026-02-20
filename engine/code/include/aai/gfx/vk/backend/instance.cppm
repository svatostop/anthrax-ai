module;
#include "aai/gfx/vk/backend/vk_defines.h"

export module aai.gfx.vk.instance;

export {
    namespace vk {
        class instance {
            public:
                void init(bool validate);
            private:
                bool enum_validation_layer_support();
                bool enum_instance_ext_support();

                VkInstance vk_instance;

                VkDebugUtilsMessengerEXT debug_messenger;
        };
    }
};

