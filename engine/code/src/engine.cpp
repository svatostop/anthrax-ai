module aai;
import aai.utils.json;
import aai.utils.mem;

void aai::core::init()
{
    win.init();
    keeper::entity_id id = keeper.create<keeper::camera>();
    gfx.set_camera(std::static_pointer_cast<keeper::camera>(keeper.get(id)));
    gfx.init(win.get_glfw_win(), win.get_display(), win.get_x11_win());
    aai::json::parse(gfx);

    // gfx.populate_inst(keeper.get_test(0));
    // gfx.populate_inst(keeper.get_test(1));
    // gfx.populate_inst(keeper.get_test(2));

    gfx.populate_base();
    gfx.init_editor(win.get_glfw_win(), editor);

    thread.start(std::bind(&aai::core::render_update_callback, this, std::placeholders::_1), std::bind(&aai::core::logic_update_callback, this, std::placeholders::_1));
}

void aai::core::render_update_callback(float delta_ms)
{
    editor.run();
    gfx.run(delta_ms);
}

void aai::core::logic_update_callback(float delta_ms)
{
    keeper.update(delta_ms);
}

void aai::core::run()
{
    while (!win.closed()) {
        time.update();
        win.poll_events();
        time.next_frame();
    }

    thread.stop();
}

void aai::core::clean()
{
    utils::mem::get()->flush_all(utils::mem::event::DELETE);
    win.clean();
}
