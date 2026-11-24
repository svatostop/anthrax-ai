module;
#include "aai/utils/lookup_table.h"
#include <ft2build.h>
#include FT_FREETYPE_H

export module aai.gfx.font;

export import std;
export import glm;

export {
    namespace ui {

        namespace alignment {
#define ALIGNMENT_LOOKUP(X) \
            X(LEFT, "left") \
            X(CENTER, "center") \
            X(SIZE, "alignment size")
DECLARE_LOOKUP_TABLE(ALIGNMENT_LOOKUP, val)
        }
    class font {
        public:
            font();
            struct context {
                unsigned int size = 0;
                float scale = 1.0f;
                glm::vec2 pos = { 0.0f, 0.0f };
                float size_x = 0.0f;
                ui::alignment::val align = ui::alignment::val::LEFT;
                std::string text = "";
            };
            struct character {
                unsigned int atlas_id;
                glm::vec4    uv;
                glm::ivec2   size;
                glm::ivec2   bearing;
                float px_face_size;
                unsigned int advance;
            };
            using fonts_map = std::unordered_map<std::string, std::map<char, character>> ;

            void load(const std::string& font_name, unsigned int atlas_id, std::function<glm::vec4(unsigned int texture_wdith, unsigned int texture_height, unsigned char* buffer)>&& add_glyph);

            const character& get_char(const std::string& font_name, char c) { 
                fonts_map::iterator it = fonts.find(font_name); 
                if (it != fonts.end()) {
                    return it->second[c];
                }
                return EMPTY_CHARACTER;
            }
            
            context* get_context() { return &ctx; }
        private:
            context ctx;

            fonts_map fonts;
            character EMPTY_CHARACTER;

    };
    }
};
