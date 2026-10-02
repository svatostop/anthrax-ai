module aai;
import aai.utils.json;
import aai.utils.mem;
import aai.utils.timer;

void aai::core::init()
{
    win.init();
    keeper::entity_id id = keeper.create<keeper::camera>();
    gfx.set_camera(std::static_pointer_cast<keeper::camera>(keeper.get(id)));
    gfx.init(win.get_glfw_win(), win.get_display(), win.get_x11_win());
    aai::json::parse(gfx);
    gfx.populate();
    gfx.init_editor(win.get_glfw_win(), editor);
}

void aai::core::run()
{
    while (!win.closed()) {
        win.poll_events();
        utils::timer::update();
        keeper.update();
        editor.run();
        gfx.run();
        utils::timer::next_frame();
    }
}

void aai::core::clean()
{
    utils::mem::get()->flush_all(utils::mem::event::DELETE);
    win.clean();
}
