import aai;
import aai.utils.mem;

void aai::core::init()
{
    win.init();
    gfx.init(win.get_display(), win.get_x11_win());
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
