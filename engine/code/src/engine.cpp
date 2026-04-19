import aai;
import aai.utils.mem;

void aai::core::init()
{
    win.init();
    renderer.init(win.get_display(), win.get_x11_win());
    renderer.populate();
}

void aai::core::run()
{
    while (!win.closed()) {
        win.poll_events();

        renderer.run();
    } 
}

void aai::core::clean()
{
    utils::mem::get()->flush(utils::mem::event::DELETE, utils::mem::type::VK);       
    win.clean();
}
