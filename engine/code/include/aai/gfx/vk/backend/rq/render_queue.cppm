export module aai.gfx.vk.rq;
import aai.gfx.attachment_ref;
import aai.gfx.materials;
import std;
export {
    namespace vk {
        namespace rq {
            struct data {
                std::string tag;
                rt::attachment_ref::info attachments;
                mat::data* material_handle = nullptr;
            };
        }
    }
};

