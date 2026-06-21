module aai;
import aai.utils.mem;

void aai::core::init()
{
    win.init();
    keeper.init(glm::vec3(-2.0f, 0.0f, -10.0f));

    gfx.set_camera(keeper.get_camera(keeper::camera::type::EDITOR));
    gfx.init(win.get_glfw_win(), win.get_display(), win.get_x11_win());
    gfx.populate();
}

void aai::core::run()
{
    while (!win.closed()) {
        win.poll_events();

        gfx.run();
    }
}

void aai::core::clean()
{
    utils::mem::get()->flush_all(utils::mem::event::DELETE);//, utils::mem::type::VK);       
    win.clean();
}
