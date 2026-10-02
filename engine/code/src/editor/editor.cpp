
module aai.editor;

import aai.editor.imgui;

void aai::editor::run()
{
    imgui.new_frame();

    imgui.show_demo_window();

    imgui.render();
}
