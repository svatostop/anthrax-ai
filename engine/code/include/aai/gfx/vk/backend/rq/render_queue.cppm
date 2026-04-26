export module aai.gfx.vk.rq;
import aai.gfx.attachments;
import aai.gfx.materials;
import std;
export {
    namespace vk {
        namespace rq {
            struct data {
                std::string tag;
                rt::attachments::ref attachments;
                mat::data* material_handle = nullptr;
            };
        }
    }
};

