module aai;
import aai.utils.mem;
import aai.utils.timer;

void aai::core::init()
{
    win.init();
    keeper.create<keeper::camera>();
    keeper::entity_id id = keeper.get_last_id();
    gfx.set_camera(std::static_pointer_cast<keeper::camera>(keeper.get(id)));
    gfx.init(win.get_glfw_win(), win.get_display(), win.get_x11_win());
    gfx.populate();
}

void aai::core::run()
{
    while (!win.closed()) {
        win.poll_events();
        keeper.update();
        utils::timer::next_frame();
        gfx.run();
    }
}

void aai::core::clean()
{
    utils::mem::get()->flush_all(utils::mem::event::DELETE);//, utils::mem::type::VK);       
    win.clean();
}
