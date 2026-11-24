module;
#include <ft2build.h>
#include FT_FREETYPE_H

module aai.gfx.font;

import aai.utils;

ui::font::font()
{
    ctx.size = 36;
    ctx.scale = 1.0f;
    ctx.size_x = 500.0f;
    ctx.pos = glm::vec2(500, 100);
}

void ui::font::load(const std::string& font_name, unsigned int atlas_id, std::function<glm::vec4(unsigned int texture_wdith, unsigned int texture_height, unsigned char* buffer)>&& add_glyph)
{
    fonts_map::iterator it = fonts.find(font_name); 
    if (it != fonts.end()) {
        utils::CHECK(true, "font::load(): font: [" + font_name + "] already loaded");
        return;
    }
    FT_Library ft;
    FT_Face face;
    utils::ASSERT(FT_Init_FreeType(&ft), "font::load(): couldn't init freetype lib");
    
    std::string path = "textures/fonts/" + font_name;
    utils::ASSERT(FT_New_Face(ft, path.c_str(), 0, &face), "font::load(): failed to load font: " + font_name);
    FT_Set_Pixel_Sizes(face, 0, ctx.size);
    
    std::map<char, character>& chars = fonts[font_name];
    for (unsigned char c = 0; c < 128; c++) {
        utils::ASSERT(FT_Load_Char(face, c, FT_LOAD_RENDER), "font::load(): failed to load glyph: " + font_name);
        glm::vec4 uv = glm::vec4(0);
        if (face->glyph->bitmap.buffer)
            uv = add_glyph(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
        character& ch = chars[c];
        ch.atlas_id = atlas_id;
        ch.uv = uv;
        ch.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        ch.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        ch.advance = face->glyph->advance.x;
        ch.px_face_size = face->size->metrics.ascender / 64.0f;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

