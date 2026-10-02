
export module aai.utils.json;
import aai.utils.json.helper;
import aai.gfx;
import aai.utils.json.materials;

export {
    namespace aai {
        namespace json {
            void parse(gfx::base& gfx) {
                parse_material_data(gfx.get_material_info_data());
            }
        }
    }
};
